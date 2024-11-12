#ifndef EPOS_INTERNAL_H
#define EPOS_INTERNAL_H

#include <pthread.h>
#include <stddef.h>
#include <time.h>
#include <semaphore.h>

#define __get_error_buf(sizep)			\
	({					\
		static char __buf[16];		\
		*(sizep) = sizeof(__buf);	\
		__buf;				\
	})

const char *symerror(int errnum);

void evl_thread_harden(void);

void evl_thread_relax(void);

const char* get_process_name(const char* command);

void rand_str(char *dest, size_t length);

extern const char* evl_program_basename;

clockid_t get_evl_clock_id(clockid_t clock_id);

struct evl_event *get_evl_cond(pthread_cond_t *std_cond);

struct evl_mutex *get_evl_mutex(pthread_mutex_t *std_mutex);

struct evl_sem *get_evl_sem(sem_t *std_sem);

int get_evl_thread_fd(pthread_t thread);


#endif