#ifndef _EPOS_WRAPPERS_H
#define _EPOS_WRAPPERS_H

#define __stringify_1(x...)	#x
#define __stringify(x...)	__stringify_1(x)

#define __WRAP(call)		__wrap_ ## call
#define __STD(call)		__real_ ## call
#define __EPOS(call)		__epos_ ## call
#define __RT(call)		__EPOS(call)
#define EPOS_DECL(T, P)	\
	__typeof__(T) __RT(P);	\
	__typeof__(T) __STD(P); \
	__typeof__(T) __WRAP(P)

#define EPOS_IMPL(T, I, A)								\
__typeof__(T) __wrap_ ## I A __attribute__((alias("__epos_" __stringify(I)), weak));	\
__typeof__(T) __epos_ ## I A

#endif