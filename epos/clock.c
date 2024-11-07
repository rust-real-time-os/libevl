#include "evl/thread.h"
#include <evl/clock.h>
#include <epos/time.h>
#include <epos/unistd.h>
#include "internal.h"

#define TIMESPEC_GREATER(t1, t2)                                               \
	((t1.tv_sec > t2.tv_sec) ||                                            \
	 (t1.tv_sec == t2.tv_sec && t1.tv_nsec > t2.tv_nsec))
#define ONE_BILLION 1000000000

EPOS_IMPL(int, clock_getres, (clockid_t clock_id, struct timespec *tp))
{
	clockid_t evl_clock_id;

	evl_clock_id = get_evl_clock_id(clock_id);

	return evl_get_clock_resolution(evl_clock_id, tp);
}

EPOS_IMPL(int, clock_gettime, (clockid_t clock_id, struct timespec *tp))
{
	clockid_t evl_clock_id = get_evl_clock_id(clock_id);

	return evl_read_clock(evl_clock_id, tp);
}

EPOS_IMPL(int, clock_adjtime, (clockid_t clock_id, struct timex *tx))
{
	//TODO: evl_adjust_timers, but don not have a user api yet.
	return -ENOTSUP;
}

EPOS_IMPL(int, clock_nanosleep,
	  (clockid_t clock_id, int flags, const struct timespec *rqtp,
	   struct timespec *rmtp))
{
	int evl_clock_id;
	struct timespec now, next;
	int carry_sec;

	switch (clock_id) {
	case CLOCK_REALTIME:
	case CLOCK_MONOTONIC_RAW:
		evl_clock_id = EVL_CLOCK_REALTIME;
		break;
	case CLOCK_MONOTONIC:
		evl_clock_id = EVL_CLOCK_MONOTONIC;
		break;
	default:
		return -ENOTSUP;
	}

	if (rqtp->tv_sec < 0 || rqtp->tv_nsec >= ONE_BILLION) {
		return -EINVAL;
	}

	if (flags & ~TIMER_ABSTIME)
		return -EINVAL;

	evl_read_clock(evl_clock_id, &now);

	if (TIMER_ABSTIME == flags) {
		next = *rqtp;
		if (!TIMESPEC_GREATER(next, now)) {
			return 0;
		}
	} else {
		carry_sec =
			((now.tv_nsec + rqtp->tv_nsec) > ONE_BILLION) ? 1 : 0;
		next.tv_nsec = (now.tv_nsec + rqtp->tv_nsec) % ONE_BILLION;
		next.tv_sec = now.tv_sec + rqtp->tv_sec + carry_sec;
	}

	return evl_sleep_until(evl_clock_id, &next);
}

EPOS_IMPL(time_t, time, (time_t * t))
{
	struct timespec ts;
	int ret = __WRAP(clock_gettime(CLOCK_REALTIME, &ts));
	if (ret)
		return (time_t)-1;

	if (t)
		*t = ts.tv_sec;
	return ts.tv_sec;
}

EPOS_IMPL(int, nanosleep, (const struct timespec *rqtp, struct timespec *rmtp))
{
	int ret;

	ret = __WRAP(clock_nanosleep(CLOCK_REALTIME, 0, rqtp, rmtp));
	if (ret) {
		errno = ret;
		return -1;
	}

	return 0;
}

EPOS_IMPL(unsigned int, sleep, (unsigned int seconds))
{
	int ret;
	if (evl_is_inband()){
		return __STD(sleep(seconds)); 
	}
	struct timespec rqt, rem;
	rqt.tv_sec = seconds;
	rqt.tv_nsec = 0;
	ret = __WRAP(clock_nanosleep(CLOCK_MONOTONIC, 0, &rqt, &rem));
	if (ret)
		return rem.tv_sec;

	return 0;
}

// EPOS_IMPL(int, timer_create,
// 	  (clockid_t clockid, const struct sigevent *__restrict__ evp,
// 	   timer_t *__restrict__ timerid))
// {

// }
