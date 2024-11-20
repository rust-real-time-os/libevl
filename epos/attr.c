

#include <stddef.h>
#include <errno.h>
#include <pthread.h>
#include <memory.h>
#include "internal.h"
#include <epos/wrappers.h>
#include <epos/pthread.h>
#include <evl/thread.h>
#include <stdio.h>

EPOS_IMPL(int, pthread_attr_init, (pthread_attr_t *attr))
{
	__STD(pthread_attr_init)(attr);
	return pthread_attr_setstacksize(attr, EVL_STACK_DEFAULT);
}

int pthread_attr_init_ex(pthread_attr_ex_t *attr_ex)
{
	/* Start with defaulting all fields to null. */
	memset(attr_ex, 0, sizeof(*attr_ex));
	/* Merge in the default standard attribute set. */
	__EPOS(pthread_attr_init)(&attr_ex->std);
	// pthread_attr_getschedpolicy(&attr_ex->std, &policy);
	attr_ex->nonstd.sched_policy = SCHED_FIFO;
	attr_ex->nonstd.sched_param.sched_priority = 1;
	attr_ex->nonstd.flags = EVL_CLONE_PUBLIC;
	return 0;
}

int pthread_attr_destroy_ex(pthread_attr_ex_t *attr_ex)
{
	return pthread_attr_destroy(&attr_ex->std);
}


int pthread_attr_setschedpolicy_ex(pthread_attr_ex_t *attr_ex,
				   int policy)
{
	attr_ex->nonstd.sched_policy = policy;

	return 0;
}

int pthread_attr_getschedpolicy_ex(const pthread_attr_ex_t *attr_ex,
				   int *policy)
{
	*policy = attr_ex->nonstd.sched_policy;

	return 0;
}

int pthread_attr_setschedparam_ex(pthread_attr_ex_t *attr_ex,
				  const struct sched_param_ex *param_ex)
{
	attr_ex->nonstd.sched_param = *param_ex;

	return 0;
}

int pthread_attr_getschedparam_ex(const pthread_attr_ex_t *attr_ex,
				  struct sched_param_ex *param_ex)
{
	*param_ex = attr_ex->nonstd.sched_param;

	return 0;
}

int pthread_attr_getinheritsched_ex(const pthread_attr_ex_t *attr_ex,
				    int *inheritsched)
{
	return pthread_attr_getinheritsched(&attr_ex->std, inheritsched);
}

int pthread_attr_getdetachstate_ex(const pthread_attr_ex_t *attr_ex,
				   int *detachstate)
{
	return pthread_attr_getdetachstate(&attr_ex->std, detachstate);
}

int pthread_attr_setdetachstate_ex(pthread_attr_ex_t *attr_ex,
				   int detachstate)
{
	return pthread_attr_setdetachstate(&attr_ex->std, detachstate);
}

int pthread_attr_getstacksize_ex(const pthread_attr_ex_t *attr_ex,
				 size_t *stacksize)
{
	return pthread_attr_getstacksize(&attr_ex->std, stacksize);
}

int pthread_attr_setstacksize_ex(pthread_attr_ex_t *attr_ex,
				 size_t stacksize)
{
	return pthread_attr_setstacksize(&attr_ex->std, stacksize);
}

int pthread_attr_getscope_ex(const pthread_attr_ex_t *attr_ex,
			     int *scope)
{
	return pthread_attr_getscope(&attr_ex->std, scope);
}

int pthread_attr_setscope_ex(pthread_attr_ex_t *attr_ex,
			     int scope)
{
	return pthread_attr_setscope(&attr_ex->std, scope);
}