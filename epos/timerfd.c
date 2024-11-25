#include <asm-generic/errno-base.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <epos/sys/timerfd.h>
#include "internal.h"
#include <epos/wrappers.h>
#include <evl/timer.h>

EPOS_IMPL(int, timerfd_create, (int clockid, int flags))
{
	int efd;
	if (flags != 0) {
		return -EINVAL;
	}
	efd = get_evl_clock_id(clockid);
	return evl_new_timer(efd);
}

EPOS_IMPL(int, timerfd_settime,
	  (int fd, int flags, const struct itimerspec *new_value,
	   struct itimerspec *old_value))
{
	if (flags != 0) {
		return -EINVAL;
	}
	return evl_set_timer(fd, new_value,old_value);
}

EPOS_IMPL(int, timerfd_gettime, (int fd, struct itimerspec *curr_value))
{
	return evl_get_timer(fd, curr_value);
}
