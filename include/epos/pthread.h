#pragma GCC system_header
#define _XOPEN_SOURCE 600
#include_next <pthread.h>
#ifndef _EPOS_PTHREAD_H
#define _EPOS_PTHREAD_H

#include <epos/wrapper.h>

#ifdef __cplusplus
extern "C" {
#endif

EPOS_DECL(int, pthread_create(pthread_t *ptid_r,
				const pthread_attr_t *attr,
				void *(*start) (void *),
				void *arg));

EPOS_DECL(int, pthread_join(pthread_t ptid, void **retval));

#ifdef __cplusplus
}
#endif

#endif /* _EPOS_PTHREAD_H */