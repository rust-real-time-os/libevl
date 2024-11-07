#include <evl/thread.h>
#include <evl/sched.h>
#include <bits/time.h>
#include <evl/mutex.h>
#include <evl/clock.h>
#include <evl/thread.h>
#include <semaphore.h>
#include <epos/pthread.h>
#include <epos/wrappers.h>
#include <errno.h>
#include <malloc.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "internal.h"

struct pthread_iargs {
	void *(*start)(void *);
	void *arg;
	int flags;
	sem_t sync;
	int parent_prio;
	int ret;
};

#define EVL_RANDOM_THREAD_NAME_LEGNTH 15

static void *evl_thread_trampoline(void *iarg)
{
	int parent_prio, flags;
	void *(*start)(void *), *arg, *retval;
	struct pthread_iargs *iargs = (struct pthread_iargs *)(iarg);
	struct sched_param std_param;
	volatile pthread_t ptid = pthread_self();
	int ret;

	char name_buf[EVL_RANDOM_THREAD_NAME_LEGNTH + 1];
	start = iargs->start;
	arg = iargs->arg;
	parent_prio = iargs->parent_prio;
	flags = iargs->flags;

	// get a random name
	rand_str((char *)&name_buf, EVL_RANDOM_THREAD_NAME_LEGNTH);

	std_param.sched_priority = 8;
	ret = __STD(pthread_setschedparam(ptid, SCHED_FIFO, &std_param));
	if (ret)
		goto sync_with_creator;
	evl_attach_thread(flags, "%s-%s", evl_program_basename, name_buf);

sync_with_creator:
	if (std_param.sched_priority == parent_prio)
		__STD(sched_yield());

	evl_thread_harden();

	retval = start(arg);

	evl_detach_self();

	return retval;
}

EPOS_IMPL(int, pthread_create,
	  (pthread_t * ptid_r, const pthread_attr_t *attr,
	   void *(*start)(void *), void *arg))
{
	struct pthread_iargs iargs;
	int ret;
	struct timespec timeout;
	iargs.start = start;
	iargs.arg = arg;
	ret = __STD(pthread_create)(ptid_r, attr, evl_thread_trampoline,
				    &iargs);

	__STD(sem_init(&iargs.sync, 0, 0));
	if (ret) {
		__STD(sem_destroy(&iargs.sync));
		return ret;
	}

	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += 5;
	timeout.tv_nsec = 0;

	for (;;) {
		sem_timedwait(&iargs.sync, &timeout);
		if (ret && errno == EINTR)
			continue;
		if (ret == 0) {
			ret = iargs.ret;
			break;
		} else if (errno == ETIMEDOUT) {
			ret = EAGAIN;
			break;
		}
		ret = -errno;
		printf("regular sem_wait failed with %s", symerror(ret));
		exit(1);
	}

	__STD(sem_destroy(&iargs.sync));

	evl_thread_harden();
	return ret;
}

EPOS_IMPL(int, pthread_join, (pthread_t ptid, void **retval))
{
	return 0;
}
EPOS_IMPL(int, pthread_kill, (pthread_t ptid, int sig))
{
	return 0;
}

EPOS_IMPL(int, pthread_yield, (void))
{
	return evl_yield();
}
