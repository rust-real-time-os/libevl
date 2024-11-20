#include <evl/thread.h>
#include <epos/pthread.h>
#include "internal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_NAME_BUF_LENGTH 1024
#define PID_STRING_LENGTH 12
#define MAX_NAME_PREFIX_LENGTH (MAX_NAME_BUF_LENGTH - PID_STRING_LENGTH - 1)

static char namebuf[MAX_NAME_BUF_LENGTH];

void set_program_name(const char *name);

static void epos_init(const char *process_name)
{
	
	struct evl_sched_attrs evl_sched_attr;
	main_thread_efd = evl_attach_thread(EVL_CLONE_PUBLIC, process_name);

	if (main_thread_efd < 0) {
		perror("Can not attach main thread\n");
		exit(-1);
	}

	evl_sched_attr.sched_policy = SCHED_FIFO;
	evl_sched_attr.sched_priority = 1;
	evl_set_schedattr(main_thread_efd,&evl_sched_attr);
	epos_thread_init();
	epos_mutex_init();
}

int epos_main(int argc, char *const argv[]);

int __real_main(int argc, char *const argv[]);

int __wrap_main(int argc, char *const argv[])
	__attribute__((alias("epos_main"), weak));

void set_program_name(const char *name)
{
	const char *start_pos;
	start_pos = strrchr(name, '/');
	if (start_pos) {
		start_pos++;
	} else {
		start_pos = name;
	}
	sprintf(namebuf, "%s-%d", start_pos,getpid());
	evl_program_basename = namebuf;
}

int epos_main(int argc, char *const argv[])
{
	set_program_name(argv[0]);
	epos_init(evl_program_basename);
	get_current_state(main_thread_efd);
	printf("aaaa\n");
	return __real_main(argc, argv);
}