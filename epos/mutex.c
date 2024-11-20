#include <eshi/evl/clock.h>
#include <evl/mutex.h>
#include <epos/pthread.h>
#include <epos/wrappers.h>
#include <assert.h>
#include "internal.h"
#include <stdio.h>

#define EVL_RANDOM_MUTEX_NAME_LEGNTH 15

static_assert(sizeof(pthread_mutex_t) >= sizeof(struct evl_mutex),
	      "size of pthread_mutex_t must greater than evl_mutex");

pthread_mutexattr_t default_pthread_mutexattr;

EPOS_IMPL(int, pthread_mutex_init,
	  (pthread_mutex_t * std_mutex, const pthread_mutexattr_t *attr))
{
	char name_buf[EVL_RANDOM_MUTEX_NAME_LEGNTH + 1];
	int mutex_type, mutex_protocol, mutex_prioceiling;
	int err;
	struct evl_mutex *mutex = get_evl_mutex(std_mutex);
	if (attr == NULL) {
		attr = &default_pthread_mutexattr;
	}

	err = pthread_mutexattr_gettype(attr, &mutex_type);
	if (err) {
		return err;
	}
	switch (mutex_type) {
	case PTHREAD_MUTEX_DEFAULT:
		mutex_type = EVL_MUTEX_NORMAL;
		break;
	case PTHREAD_MUTEX_RECURSIVE:
		mutex_type = EVL_MUTEX_RECURSIVE;
	default:
		return -EINVAL;
	}

	err = pthread_mutexattr_getprotocol(attr, &mutex_protocol);
	if (err) {
		return err;
	}

	if (mutex_protocol == PTHREAD_PRIO_PROTECT) {
		// We do not check it here. The `evl_create_mutex` will check
		// if the ceiling is valid.
		err = pthread_mutexattr_getprioceiling(attr,
						       &mutex_prioceiling);
		if (err)
			return err;
	} else {
		mutex_prioceiling = 0;
	}

	rand_str((char *)&name_buf, EVL_RANDOM_MUTEX_NAME_LEGNTH);
	name_buf[EVL_RANDOM_MUTEX_NAME_LEGNTH] = 0;
	err = evl_create_mutex(mutex, EVL_CLOCK_MONOTONIC, mutex_prioceiling,
				mutex_type | EVL_CLONE_PRIVATE, "evl_mutex-%s",
				name_buf);
	return (err >= 0) ? 0: -err;
}

EPOS_IMPL(int, pthread_mutex_lock, (pthread_mutex_t * std_mutex))
{
	struct evl_mutex *mutex = get_evl_mutex(std_mutex);
	return evl_lock_mutex(mutex);
}

EPOS_IMPL(int, pthread_mutex_unlock, (pthread_mutex_t * std_mutex))
{
	struct evl_mutex *mutex = get_evl_mutex(std_mutex);
	return evl_unlock_mutex(mutex);
}

EPOS_IMPL(int, pthread_mutex_destroy, (pthread_mutex_t * std_mutex))
{
	struct evl_mutex *mutex = get_evl_mutex(std_mutex);
	return evl_close_mutex(mutex);
}

EPOS_IMPL(int, pthread_mutex_trylock, (pthread_mutex_t * std_mutex))
{
	struct evl_mutex *mutex = get_evl_mutex(std_mutex);
	return evl_trylock_mutex(mutex);
}

EPOS_IMPL(int, pthread_mutex_timedlock,
	  (pthread_mutex_t * std_mutex, const struct timespec *timeout))
{
	struct evl_mutex *mutex = get_evl_mutex(std_mutex);
	return evl_timedlock_mutex(mutex, timeout);
}

EPOS_IMPL(int, pthread_mutex_setprioceiling,
	  (pthread_mutex_t * std_mutex, int prioceiling, int *old_ceiling))
{
	int err, old_ceiling_tmp;
	struct evl_mutex *mutex = get_evl_mutex(std_mutex);
	err = evl_get_mutex_ceiling(mutex);
	if (err >= 0) {
		old_ceiling_tmp = err;
	} else {
		return err;
	}
	err = evl_set_mutex_ceiling(mutex, prioceiling);
	if (err) {
		*old_ceiling = old_ceiling_tmp;
	}
	return err;
}

EPOS_IMPL(int, pthread_mutex_getprioceiling,
	  (pthread_mutex_t * std_mutex, int *old_ceiling))
{
	int err;
	struct evl_mutex *mutex = get_evl_mutex(std_mutex);
	err = evl_get_mutex_ceiling(mutex);
	if (err >= 0) {
		*old_ceiling = err;
		return 0;
	} else {
		return err;
	}
}

void epos_mutex_init(void)
{
	pthread_mutexattr_init(&default_pthread_mutexattr);
	pthread_mutexattr_settype(&default_pthread_mutexattr,
				  PTHREAD_MUTEX_DEFAULT);
	pthread_mutexattr_setprotocol(&default_pthread_mutexattr,
				      PTHREAD_PRIO_NONE);
	pthread_mutexattr_setprioceiling(&default_pthread_mutexattr, 0);
}