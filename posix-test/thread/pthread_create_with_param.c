#include <pthread.h>
#include <stdio.h>
#include "../helpers.h"
#include <unistd.h>
#include <evl/thread.h>

static void *sub_thread(void *_)
{
    struct evl_thread_state buf;
    int ret,efd;
    efd = evl_get_self();
	__Tcall_errno_assert(ret, evl_get_state(efd, &buf));
    __Texpr_assert(buf.eattrs.sched_policy == 1);
    __Texpr_assert(buf.eattrs.sched_priority== 2);

	return NULL;
}

int main()
{
	int ret;
	pthread_t tid;
    pthread_attr_t attr;
    struct sched_param param;
    param.sched_priority = 2;
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    pthread_attr_setschedparam(&attr, &param);
	__Tcall_errno_assert(ret, pthread_create(&tid, &attr, sub_thread, NULL));
	printf("tid = %ld\n", tid);
	__Tcall_errno_assert(ret, pthread_join(tid, NULL));
}