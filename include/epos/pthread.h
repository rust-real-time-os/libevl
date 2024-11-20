#pragma GCC system_header
#define _XOPEN_SOURCE 600
#include_next <pthread.h>
#ifndef _EPOS_PTHREAD_H
#define _EPOS_PTHREAD_H

#include <epos/wrappers.h>
#include <evl/sched.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sched_param_ex {
	int sched_priority;
	union {
		struct __evl_rr_param rr;
		struct __evl_quota_param quota;
		struct __evl_tp_param tp;
	} sched_u;
};
typedef struct pthread_attr_ex{
	pthread_attr_t std;
	struct {
		struct sched_param_ex sched_param;
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

// EPOS_DECL(int, pthread_kill(pthread_t ptid, int sig));

EPOS_DECL(int, pthread_join(pthread_t ptid, void **retval));

EPOS_DECL(int, pthread_yield(void));

// EPOS_DECL(int, pthread_setname_np(pthread_t thread, const char *name));

#ifdef __cplusplus
}
#endif

void epos_thread_init(void);

void epos_mutex_init(void);

// void epos_cond_init(void);

int pthread_create_ex(pthread_t *ptid_r, const pthread_attr_ex_t *attr_ex,
		      void *(*start)(void *), void *arg);

int pthread_attr_init_ex(pthread_attr_ex_t *attr_ex);

int pthread_attr_destroy_ex(pthread_attr_ex_t *attr_ex);

int pthread_getschedparam_ex(pthread_t ptid,
			     int *pol,
			     struct sched_param_ex *par);

int pthread_setschedparam_ex(pthread_t ptid,
			     int pol,
			     const struct sched_param_ex *par);

int pthread_attr_setschedpolicy_ex(pthread_attr_ex_t *attr_ex,
				   int policy);

int pthread_attr_getschedpolicy_ex(const pthread_attr_ex_t *attr_ex,
				   int *policy);

int pthread_attr_setschedparam_ex(pthread_attr_ex_t *attr_ex,
				  const struct sched_param_ex *param_ex);

int pthread_attr_getschedparam_ex(const pthread_attr_ex_t *attr_ex,
				  struct sched_param_ex *param_ex);

int pthread_attr_getinheritsched_ex(const pthread_attr_ex_t *attr_ex,
				    int *inheritsched);

int pthread_attr_setinheritsched_ex(pthread_attr_ex_t *attr_ex,
				    int inheritsched);

int pthread_attr_getdetachstate_ex(const pthread_attr_ex_t *attr_ex,
				   int *detachstate);

int pthread_attr_setdetachstate_ex(pthread_attr_ex_t *attr_ex,
				   int detachstate);

int pthread_attr_setdetachstate_ex(pthread_attr_ex_t *attr_ex,
				   int detachstate);

int pthread_attr_getstacksize_ex(const pthread_attr_ex_t *attr_ex,
				 size_t *stacksize);

int pthread_attr_setstacksize_ex(pthread_attr_ex_t *attr_ex,
				 size_t stacksize);

int pthread_attr_getscope_ex(const pthread_attr_ex_t *attr_ex,
			     int *scope);

int pthread_attr_setscope_ex(pthread_attr_ex_t *attr_ex,
			     int scope);

#endif /* _EPOS_PTHREAD_H */