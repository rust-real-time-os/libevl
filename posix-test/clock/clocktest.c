/*
 * Copyright (C) 2007 Jan Kiszka <jan.kiszka@web.de>.
 *
 * Xenomai is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Xenomai is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Xenomai; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <error.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>

extern struct xnvdso *cobalt_vdso;

/*
 * We can't really trust POSIX headers to check for features, since
 * some archs may not implement all of the declared uClibc POSIX
 * features (e.g. NIOS2).
 */
#ifdef HAVE_PTHREAD_SPIN_LOCK
pthread_spinlock_t lock;
#define init_lock(lock)				pthread_spin_init(lock, 0)
#define acquire_lock(lock)			pthread_spin_lock(lock)
#define release_lock(lock)			pthread_spin_unlock(lock)
#else
pthread_mutex_t lock;
#define init_lock(lock)				pthread_mutex_init(lock, NULL)
#define acquire_lock(lock)			pthread_mutex_lock(lock)
#define release_lock(lock)			pthread_mutex_unlock(lock)
#endif

static uint64_t last_common = 0;
static clockid_t clock_id = CLOCK_REALTIME;
static cpu_set_t cpu_realtime_set, cpu_online_set;

struct per_cpu_data {
	uint64_t first_tod, first_clock;
	int first_round;
	int64_t offset;
	double drift;
	unsigned long warps;
	uint64_t max_warp;
	pthread_t thread;
} *per_cpu_data;

// static void show_realtime_offset(void)
// {
// 	if (!xnvdso_test_feature(cobalt_vdso, XNVDSO_FEAT_HOST_REALTIME)) {
// 		printf("XNVDSO_FEAT_WALLCLOCK_OFFSET not available\n");
// 		return;
// 	}

// 	printf("Wallclock offset : %llu\n", (long long)cobalt_vdso->wallclock_offset);
// }

static inline uint64_t read_clock(clockid_t clock_id)
{
	struct timespec ts;
	int res;

	res = clock_gettime(clock_id, &ts);
	if (res != 0) {
		fprintf(stderr, "clock_gettime failed for clock id %d\n",
			clock_id);
		// if (clock_id == CLOCK_REALTIME)
		// 	show_realtime_offset();

		exit(-1);
	}
	return ts.tv_nsec + ts.tv_sec * 1000000000ULL;
}

static inline uint64_t read_reference_clock(void)
{
	struct timeval tv;

	/*
	 * Make sure we do not pick the vsyscall variant. It won't
	 * switch us into secondary mode and can easily deadlock.
	 */
	syscall(SYS_gettimeofday, &tv, NULL);
	return tv.tv_usec * 1000ULL + tv.tv_sec * 1000000000ULL;
}

static void check_reference(struct per_cpu_data *per_cpu_data)
{
	uint64_t clock_val[10], tod_val[10];
	int64_t delta, min_delta;
	int i, idx;

	for (i = 0; i < 10; i++) {
		tod_val[i] = read_reference_clock();
		clock_val[i] = read_clock(clock_id);
	}

	min_delta = tod_val[1] - tod_val[0];
	idx = 1;

	for (i = 2; i < 10; i++) {
		delta = tod_val[i] - tod_val[i-1];
		if (delta < min_delta) {
			min_delta = delta;
			idx = i;
		}
	}

	if (per_cpu_data->first_round) {
		per_cpu_data->first_round = 0;

		per_cpu_data->first_tod = tod_val[idx];
		per_cpu_data->first_clock = clock_val[idx];
	} else
		per_cpu_data->drift =
			(clock_val[idx] - per_cpu_data->first_clock) /
			(double)(tod_val[idx] - per_cpu_data->first_tod) - 1;

	per_cpu_data->offset = clock_val[idx] - tod_val[idx];
}

static void check_time_warps(struct per_cpu_data *per_cpu_data)
{
	int i;
	uint64_t last, now;
	int64_t incr;

	for (i = 0; i < 100; i++) {
		acquire_lock(&lock);
		now = read_clock(clock_id);
		last = last_common;
		last_common = now;
		release_lock(&lock);

		incr = now - last;
		if (incr < 0) {
			acquire_lock(&lock);
			per_cpu_data->warps++;
			if (-incr > per_cpu_data->max_warp)
				per_cpu_data->max_warp = -incr;
			release_lock(&lock);
		}
	}
}

static void *cpu_thread(void *arg)
{
	int cpuid = (long)arg;
	struct sched_param param = { .sched_priority = 1 };
	struct timespec delay = { 0, 0 };
	cpu_set_t cpu_set;

	srandom(read_reference_clock());

	CPU_SET(cpuid, &cpu_set);
	sched_setaffinity(0, sizeof(cpu_set), &cpu_set);
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);

	while (1) {
		check_reference(&per_cpu_data[cpuid]);

		check_time_warps(&per_cpu_data[cpuid]);

		delay.tv_nsec = 1000000 + random() * (100000.0 / RAND_MAX);
		nanosleep(&delay, NULL);
	}

	return NULL;
}

static void sighand(int signal)
{
	exit(0);
}


int main(int argc, char *argv[])
{
	const char *clock_name = NULL, *real_clock_name = "CLOCK_REALTIME";
	int max_cpu, cpus;
	int i;
	int c;
	int ext = 0;

	while ((c = getopt(argc, argv, "C:ET:D")) != EOF)
		switch (c) {
		case 'C':
			clock_name = optarg;
			break;

		case 'E':
			ext = 1;
			break;

		case 'T':
			alarm(atoi(optarg));
			break;

		default:
			fprintf(stderr, "usage: clocktest [options]\n"
				"  [-C <clock_id|clock_name>]   # tested clock, defaults to CLOCK_REALTIME\n"
				"  [-E]                         # -C specifies extension clock\n"
				"  [-T <test_duration_seconds>] # default=0, so ^C to end\n"
				"  [-D]                         # print extra diagnostics for CLOCK_HOST_REALTIME\n");
			exit(2);
		}

	// if (clock_name)
	// 	clock_id = resolve_clock_name(clock_name, &real_clock_name, ext);
    clock_id = CLOCK_REALTIME;

	signal(SIGALRM, sighand);

	init_lock(&lock);

	// if (d && clock_id == CLOCK_HOST_REALTIME)
	// 	show_hostrt_diagnostics();

	// if (get_realtime_cpu_set(&cpu_realtime_set) != 0)
	// 	error(1, ENOSYS, "get_realtime_cpu_set");

	// if (get_online_cpu_set(&cpu_online_set) != 0)
	// 	error(1, ENOSYS, "get_online_cpu_set");

	// CPU_AND(&cpu_realtime_set, &cpu_realtime_set, &cpu_online_set);

	max_cpu = 4;
	cpus = 4;
	// for (i = 0; i < CPU_SETSIZE; i++) {
	// 	if (!CPU_ISSET(i, &cpu_realtime_set))
	// 		continue;
	// 	cpus++;
	// 	if (i > max_cpu)
	// 		max_cpu = i;
	// }

	per_cpu_data = malloc(sizeof(*per_cpu_data) * (max_cpu + 1));
	if (per_cpu_data == NULL)
		error(1, ENOMEM, "malloc");

	memset(per_cpu_data, 0, sizeof(*per_cpu_data) * (max_cpu + 1));

	for (i = 0; i <= max_cpu; i++) {
		// if (!CPU_ISSET(i, &cpu_realtime_set))
		// 	continue;
		per_cpu_data[i].first_round = 1;
		pthread_create(&per_cpu_data[i].thread, NULL, cpu_thread,
			       (void *)(long)i);
	}

	printf("== Testing %s %s (%d)\n",
	       ext ? "extension" : "built-in", real_clock_name, clock_id);
	printf("CPU      ToD offset [us] ToD drift [us/s]      warps max delta [us]\n"
	       "--- -------------------- ---------------- ---------- --------------\n");

	while (1) {
		for (i = 0; i <= max_cpu; i++)
			// if (CPU_ISSET(i, &cpu_realtime_set))
            printf("%3d %20.1f %16.3f %10lu %14.1f\n",
                    i,
                    per_cpu_data[i].offset/1000.0,
                    per_cpu_data[i].drift * 1000000.0,
                    per_cpu_data[i].warps,
                    per_cpu_data[i].max_warp/1000.0);
		usleep(250000);
		printf("\033[%dA", cpus);
	}
}
