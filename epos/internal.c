#include <evl/thread.h>
#include <evl/clock.h>
#include <errno.h>
#include "internal.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

const char *evl_program_basename = NULL;

int main_thread_efd = -1;

#define __esym_def(e) [e] = #e

static const char *__esym_map[] = {
	[0] = "OK",
	__esym_def(EPERM),
	__esym_def(ENOENT),
	__esym_def(ESRCH),
	__esym_def(EINTR),
	__esym_def(EIO),
	__esym_def(ENXIO),
	__esym_def(E2BIG),
	__esym_def(ENOEXEC),
	__esym_def(EBADF),
	__esym_def(ECHILD),
	__esym_def(EAGAIN),
	__esym_def(ENOMEM),
	__esym_def(EACCES),
	__esym_def(EFAULT),
	__esym_def(ENOTBLK),
	__esym_def(EBUSY),
	__esym_def(EEXIST),
	__esym_def(EXDEV),
	__esym_def(ENODEV),
	__esym_def(ENOTDIR),
	__esym_def(EISDIR),
	__esym_def(EINVAL),
	__esym_def(ENFILE),
	__esym_def(EMFILE),
	__esym_def(ENOTTY),
	__esym_def(ETXTBSY),
	__esym_def(EFBIG),
	__esym_def(ENOSPC),
	__esym_def(ESPIPE),
	__esym_def(EROFS),
	__esym_def(EMLINK),
	__esym_def(EPIPE),
	__esym_def(EDOM),
	__esym_def(ERANGE),
	__esym_def(ENOSYS),
	__esym_def(ETIMEDOUT),
	__esym_def(ENOMSG),
	__esym_def(EIDRM),
	__esym_def(EADDRINUSE),
	__esym_def(EPROTO),
};

#define __esym_max (sizeof(__esym_map) / sizeof(__esym_map[0]))

const char *symerror(int errnum)
{
	int v = -errnum;
	size_t ebufsz;
	char *ebuf;

	if (v < 0 || v >= (int)__esym_max || __esym_map[v] == NULL) {
		/* Catch missing codes in the error map. */
		ebuf = __get_error_buf(&ebufsz);
		snprintf(ebuf, ebufsz, "%d?", errnum);
		return ebuf;
	}

	return __esym_map[v];
}

void rand_str(char *dest, size_t length)
{
	char charset[] = "0123456789"
			 "abcdefghijklmnopqrstuvwxyz"
			 "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	while (length-- > 0) {
		size_t index = (double)rand() / RAND_MAX * (sizeof charset - 1);
		*dest++ = charset[index];
	}
	*dest = '\0';
}

void evl_thread_harden(void)
{
	if (evl_is_inband()) {
		evl_switch_oob();
	}
}

void evl_thread_relax(void)
{
	if (!evl_is_inband()) {
		evl_switch_inband();
	}
}

clockid_t get_evl_clock_id(clockid_t clock_id)
{
	switch (clock_id) {
	case CLOCK_REALTIME:
		return EVL_CLOCK_REALTIME;
	case CLOCK_MONOTONIC:
	case CLOCK_MONOTONIC_RAW:
		return EVL_CLOCK_MONOTONIC;
	default:
		return clock_id;
	}
}

struct evl_mutex *get_evl_mutex(pthread_mutex_t *std_mutex)
{
	return (struct evl_mutex *)std_mutex;
}

struct evl_event *get_evl_cond(pthread_cond_t *std_cond)
{
	return (struct evl_event *)std_cond;
}

struct evl_sem *get_evl_sem(sem_t *std_sem)
{
	return (struct evl_sem *)std_sem;
}

void set_current_thread_debug_mode(int efd)
{
	const int mask = T_WOSS | T_WOLI;
	int err;
	err = evl_set_thread_mode(efd, mask,NULL);
	if (err) {
		printf("Fail to set thread mode,err=%d", err);
	}
}

void get_current_state(int efd)
{
	struct evl_thread_state buf;
	int err;
	err = evl_get_state(efd, &buf);
	if (err) {
		printf("Failed to get state,err=%d", err);
	}

	printf("sched_policy=%d,sched_priority=%d,state=%x\n",
	       buf.eattrs.sched_policy, buf.eattrs.sched_priority, buf.state);
}