#include <epos/pthread.h>
#include <epos/wrapper.h>
#include <errno.h>
#include <evl/thread.h>
#include <malloc.h>
struct pthread_iargs {
  void *(*start)(void *);
  void *arg;
};

static void *evl_thread_trampoline(void *arg) {
  void *ret;
  struct pthread_iargs *params = (struct pthread_iargs *)(arg);
  evl_attach_self("uthread");
  ret = params->start(params->arg);
  evl_detach_self();
  return ret;
}

EPOS_IMPL(int, pthread_create,
          (pthread_t * ptid_r, const pthread_attr_t *attr,
           void *(*start)(void *), void *arg)) {
  struct pthread_iargs *iargs = malloc(sizeof(struct pthread_iargs));
  if (iargs == NULL)
    return -ENOMEM;
  iargs->start = start;
  iargs->arg = arg;
  __STD(pthread_create)(ptid_r, attr, evl_thread_trampoline, iargs);
  return 0;
}