#include <epos/pthread.h>

__attribute__((weak))
int __real_pthread_create(pthread_t *ptid_r,
			  const pthread_attr_t * attr,
			  void *(*start) (void *), void *arg)
{
	return pthread_create(ptid_r, attr, start, arg);
}

__attribute__((weak))
int __real_pthread_join(pthread_t ptid, void **retval)
{
	return pthread_join(ptid, retval);
}