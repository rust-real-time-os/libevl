#include<pthread.h>
#include<stdio.h>

static void* sub_thread(void*_){
    printf("run a subthread");
    return NULL;
}

int main(){
    pthread_t tid;
    int ret;
    ret = pthread_create(&tid, NULL, sub_thread, NULL);
    if (ret){
        printf("error!\n");
    }
    printf("tid = %ld\n",tid);
}