#pragma GCC system_header
#define _XOPEN_SOURCE 600
#include_next <pthread.h>
#ifndef _EPOS_PTHREAD_H
#define _EPOS_PTHREAD_H

#include <epos/wrappers.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pthread_attr_ex{
	pthread_attr_t std;
	struct {
		int personality;
		int sched_policy;
		int flags;
		char* thread_name;
	}nonstd;
}pthread_attr_ex_t;

EPOS_DECL(int, pthread_attr_init(pthread_attr_t *attr));

EPOS_DECL(int, pthread_create(pthread_t *ptid_r,
				const pthread_attr_t *attr,
				void *(*start) (void *),
				void *arg));

EPOS_DECL(int, pthread_getschedparam(pthread_t thread,
				       int *policy,
				       struct sched_param *param));

EPOS_DECL(int, pthread_setschedparam(pthread_t thread,
				       int policy,
				       const struct sched_param *param));

EPOS_DECL(int, pthread_setschedprio(pthread_t thread, int prio));

EPOS_DECL(int, pthread_mutex_init(pthread_mutex_t *mutex,
				    const pthread_mutexattr_t *attr));

EPOS_DECL(int, pthread_mutex_destroy(pthread_mutex_t *mutex));

EPOS_DECL(int, pthread_mutex_lock(pthread_mutex_t *mutex));

EPOS_DECL(int, pthread_mutex_timedlock(pthread_mutex_t *mutex,
					 const struct timespec *to));

EPOS_DECL(int, pthread_mutex_trylock(pthread_mutex_t *mutex));

EPOS_DECL(int, pthread_mutex_unlock(pthread_mutex_t *mutex));

EPOS_DECL(int, pthread_mutex_setprioceiling(pthread_mutex_t *__restrict mutex,
					      int prioceiling,
					      int *__restrict old_ceiling));
  
EPOS_DECL(int, pthread_mutex_getprioceiling(pthread_mutex_t *__restrict mutex,
					      int *__restrict old_ceiling));

EPOS_DECL(int, pthread_cond_init (pthread_cond_t *cond,
				    const pthread_condattr_t *attr));

EPOS_DECL(int, pthread_cond_destroy(pthread_cond_t *cond));

EPOS_DECL(int, pthread_cond_wait(pthread_cond_t *cond,
				   pthread_mutex_t *mutex));

EPOS_DECL(int, pthread_cond_timedwait(pthread_cond_t *cond,
					pthread_mutex_t *mutex,
					const struct timespec *abstime));

EPOS_DECL(int, pthread_cond_signal(pthread_cond_t *cond));

EPOS_DECL(int, pthread_cond_broadcast(pthread_cond_t *cond));

EPOS_DECL(int, pthread_kill(pthread_t ptid, int sig));

EPOS_DECL(int, pthread_join(pthread_t ptid, void **retval));

EPOS_DECL(int, pthread_yield(void));

EPOS_DECL(int, pthread_setname_np(pthread_t thread, const char *name));

#ifdef __cplusplus
}
#endif

#endif /* _EPOS_PTHREAD_H */