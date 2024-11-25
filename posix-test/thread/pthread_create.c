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
    printf("sched_policy=%d,sched_priority=%d,state=%d\n",
	       buf.eattrs.sched_policy, buf.eattrs.sched_priority, buf.state);
	return NULL;
}

int main()
{
	int ret;
	pthread_t tid;
	__Tcall_errno_assert(ret, pthread_create(&tid, NULL, sub_thread, NULL));
	printf("tid = %ld\n", tid);
	__Tcall_errno_assert(ret, pthread_join(tid, NULL));
}