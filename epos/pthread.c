#include <epos/pthread.h>
#include <epos/wrapper.h>
#include <errno.h>
#include <evl/thread.h>
#include <malloc.h>
#include <stdlib.h>
struct pthread_iargs {
  void *(*entry)(void *);
  void *arg;
};

#define EVL_RANDOM_THREAD_NAME_LEGNTH 15

static void rand_str(char *dest, size_t length) {
    char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    while (length-- > 0) {
        size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
        *dest++ = charset[index];
    }
    *dest = '\0';
}

static void *evl_thread_trampoline(void *iarg) {
  void *ret,*arg;
  void *(*entry)(void *);
  char name_buf[16];
  struct pthread_iargs *params = (struct pthread_iargs *)(iarg);
  entry = params->entry;
  arg = params->arg;
  free(params);

  // get a random name
  rand_str((char*)&name_buf,EVL_RANDOM_THREAD_NAME_LEGNTH);

  evl_attach_self("%s",name_buf);
  ret = entry(arg);
  evl_detach_self();
  return ret;
}

EPOS_IMPL(int, pthread_create,
          (pthread_t * ptid_r, const pthread_attr_t *attr,
           void *(*entry)(void *), void *arg)) {
  // This struct should be freed by trampoline
  struct pthread_iargs *iargs = malloc(sizeof(struct pthread_iargs));
  if (iargs == NULL)
    return -ENOMEM;
  iargs->entry = entry;
  iargs->arg = arg;
  __STD(pthread_create)(ptid_r, attr, evl_thread_trampoline, iargs);
  return 0;
}