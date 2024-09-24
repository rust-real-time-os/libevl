#pragma GCC system_header
#define _XOPEN_SOURCE 600
#include_next <pthread.h>
#ifndef _EPOS_PTHREAD_H
#define _EPOS_PTHREAD_H

#include <epos/wrapper.h>

#ifdef __cplusplus
extern "C" {
#endif

EPOS_DECL(int, pthread_create(pthread_t *ptid_r, const pthread_attr_t *attr,
                              void *(*start)(void *), void *arg));

EPOS_DECL(int, pthread_join(pthread_t ptid, void **retval));

EPOS_DECL(int, pthread_attr_setstacksize(pthread_attr_t *__attr,
                                         size_t __stacksize));
EPOS_DECL(int, pthread_attr_destroy(pthread_attr_t *__attr));

typedef struct evl_mutex evl_mutex;
#define pthread_mutex_t evl_mutex

EPOS_DECL(int, pthread_mutex_lock(pthread_mutex_t *__mutex));

EPOS_DECL(int, pthread_mutex_unlock(pthread_mutex_t *__mutex));

EPOS_DECL(int, pthread_mutex_init(pthread_mutex_t *__mutex,
                                  const pthread_mutexattr_t *__mutexattr));

// EPOS_DECL(int, pthread_mutexattr_init(pthread_mutexattr_t *__attr));

// EPOS_DECL(int, pthread_mutexattr_setprotocol(pthread_mutexattr_t *__attr,
//                                              int __protocol));

// EPOS_DECL(int, pthread_setschedparam(pthread_t __target_thread, int __policy,
//                                      const struct 


#ifdef __cplusplus
}
#endif

#endif /* _EPOS_PTHREAD_H */