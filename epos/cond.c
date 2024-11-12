#include "evl/event.h"
#include <epos/pthread.h>
#include <epos/wrappers.h>
#include <assert.h>
#include "epos_internal.h"
#include <pthread.h>
#include <errno.h>

#define EVL_RANDOM_COND_NAME_LEGNTH 15

static_assert(sizeof(pthread_cond_t) >= sizeof(struct evl_event),
	      "size of pthread_cond_t must greater than evl_event");

EPOS_IMPL(int, pthread_cond_init,
	  (pthread_cond_t * std_cond, const pthread_condattr_t *attr))
{
	int err;
	int clock_id, is_cond_pthread_shared;
	char name_buf[EVL_RANDOM_COND_NAME_LEGNTH + 1];
	struct evl_event *cond = get_evl_cond(std_cond);
	err = pthread_condattr_getclock(attr, &clock_id);
	if (err)
		return err;
	clock_id = get_evl_clock_id(clock_id);

	err = pthread_condattr_getpshared(attr, &is_cond_pthread_shared);
	if (err)
		return err;
	if (is_cond_pthread_shared) {
		// not support currently. We may try evl_open_event later.
		return -EINVAL;
	}

	rand_str((char *)&name_buf, EVL_RANDOM_COND_NAME_LEGNTH);
	return evl_create_event(cond, clock_id, EVL_CLONE_PRIVATE, "%s-cond-%s",
				evl_program_basename, name_buf);
}

EPOS_IMPL(int, pthread_cond_destroy, (pthread_cond_t * std_cond))
{
	struct evl_event *cond = get_evl_cond(std_cond);
	return evl_close_event(cond);
}

EPOS_IMPL(int, pthread_cond_wait,
	  (pthread_cond_t * std_cond, pthread_mutex_t *std_mutex))
{
	struct evl_event *cond = get_evl_cond(std_cond);
	struct evl_mutex *mutex = get_evl_mutex(std_mutex);
	return evl_wait_event(cond, mutex);
}

EPOS_IMPL(int, pthread_cond_timedwait,
	  (pthread_cond_t * std_cond, pthread_mutex_t *std_mutex,
	   const struct timespec *timeout))
{
	struct evl_event *cond = get_evl_cond(std_cond);
	struct evl_mutex *mutex = get_evl_mutex(std_mutex);
	return evl_timedwait_event(cond, mutex, timeout);
}

EPOS_IMPL(int, pthread_cond_signal, (pthread_cond_t * std_cond))
{
    struct evl_event *cond = get_evl_cond(std_cond);
    return evl_signal_event(cond);
}

EPOS_IMPL(int, pthread_cond_broadcast, (pthread_cond_t * std_cond))
{
	struct evl_event *cond = get_evl_cond(std_cond);
    return evl_broadcast_event(cond);
}