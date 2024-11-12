#include <asm-generic/errno-base.h>
#include <eshi/evl/clock.h>
#include <evl/mutex.h>
#include <epos/pthread.h>
#include <epos/wrappers.h>
#include <assert.h>
#include "epos_internal.h"
#include <pthread.h>

static_assert(sizeof(pthread_mutex_t) >= sizeof(struct evl_mutex),
	      "size of pthread_mutex_t must greater than evl_mutex");

EPOS_IMPL(int, pthread_mutex_init,
	  (pthread_mutex_t * std_mutex, const pthread_mutexattr_t *attr))
{
	int mutex_type,mutex_protocol,mutex_prioceiling;
	int err;
	struct evl_mutex *mutex = get_evl_mutex(std_mutex);
	err = pthread_mutexattr_gettype(attr,&mutex_type);
	if (err){
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

	err = pthread_mutexattr_getprotocol(attr,&mutex_protocol);
	if (err){
		return err;
	}

	if (mutex_protocol == PTHREAD_PRIO_PROTECT) {
		// We do not check it here. The `evl_create_mutex` will check
		// if the ceiling is valid.
		err = pthread_mutexattr_getprioceiling(attr, &mutex_prioceiling);
		if (err)
			return err;
	}else{
		mutex_prioceiling = 0;
	}

	return evl_create_mutex(mutex, EVL_CLOCK_MONOTONIC, 0, mutex_type,
				"evl_mutex");
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
	int ret, old_ceiling_tmp;
	struct evl_mutex *mutex = get_evl_mutex(std_mutex);
	ret = evl_get_mutex_ceiling(mutex);
	if (ret >= 0) {
		old_ceiling_tmp = ret;
	} else {
		return ret;
	}
	ret = evl_set_mutex_ceiling(mutex, prioceiling);
	if (ret) {
		*old_ceiling = old_ceiling_tmp;
	}
	return ret;
}

EPOS_IMPL(int, pthread_mutex_getprioceiling,
	  (pthread_mutex_t * std_mutex, int *old_ceiling))
{
	int ret;
	struct evl_mutex *mutex = get_evl_mutex(std_mutex);
	ret = evl_get_mutex_ceiling(mutex);
	if (ret >= 0) {
		*old_ceiling = ret;
		return 0;
	} else {
		return ret;
	}
}