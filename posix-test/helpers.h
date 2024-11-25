#ifndef _EPOS_TESTS_HELPERS_H
#define _EPOS_TESTS_HELPERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <pthread.h>
#include <linux/ioctl.h>
#include <evl/proxy.h>

#define EXIT_NO_SUPPORT  42

#define ONE_BILLION	1000000000

#define __stringify_1(x...)	#x
#define __stringify(x...)	__stringify_1(x)

#define warn_failed(__fmt, __args...)					\
	evl_print_proxy(proxy_errfd, "%s:%d: FAILED: " __fmt "\n",	\
			__FILE__, __LINE__, ##__args)

static inline int abort_test(int status)
{
	exit(status == -EOPNOTSUPP ? EXIT_NO_SUPPORT : 1);
}

#define __Tcall(__ret, __call)				\
	({						\
		(__ret) = (__call);			\
		if (__ret < 0) {			\
			warn_failed("%s (=%s)",		\
				__stringify(__call),	\
				strerror(-(__ret)));	\
		}					\
		(__ret) >= 0;				\
	})

#define __Tcall_assert(__ret, __call)		\
	do {					\
		if (!__Tcall(__ret, __call))	\
			abort_test(__ret);	\
	} while (0)

#define __Fcall(__ret, __call)				\
	({						\
		(__ret) = (__call);			\
		if ((__ret) >= 0)			\
			warn_failed("%s (%d >= 0)",	\
				__stringify(__call),	\
				__ret);			\
		(__ret) < 0;				\
	})

#define __Fcall_assert(__ret, __call)		\
	do {					\
		if (!__Fcall(__ret, __call))	\
			abort_test(__ret);	\
	} while (0)

#define __Tcall_errno(__ret, __call)			\
	({						\
		(__ret) = (__call);			\
		if (__ret < 0)				\
			warn_failed("%s (=%s)",		\
				__stringify(__call),	\
				strerror(errno));	\
		(__ret) >= 0;				\
	})

#define __Tcall_errno_assert(__ret, __call)		\
	do {						\
		(__ret) = (__call);			\
		if (__ret < 0) {			\
			int __errval = errno;		\
			warn_failed("%s (=%s)",		\
				__stringify(__call),	\
				strerror(__errval));	\
			abort_test(-__errval);		\
		}					\
	} while (0)

#define __Texpr(__expr)					\
	({						\
		int __ret = !!(__expr);			\
		if (!__ret)				\
			warn_failed("%s (=false)",	\
				__stringify(__expr));	\
		__ret;					\
	})

#define __Texpr_assert(__expr)		\
	do {				\
		if (!__Texpr(__expr))	\
			abort_test(0);	\
	} while (0)

#define __Fexpr(__expr)					\
	({						\
		int __ret = (__expr);			\
		if (__ret)				\
			warn_failed("%s (=true)",	\
				__stringify(__expr));	\
		!__ret;					\
	})

#define __Fexpr_assert(__expr)		\
	do {				\
		if (!__Fexpr(__expr))	\
			abort_test(0);	\
	} while (0)

#endif /* !_EPOS_TESTS_HELPERS_H */