#include <pthread.h>
#include <stdio.h>
#include "../helpers.h"
#include <unistd.h>
#include <evl/thread.h>


#define NUM_THREADS 5
#define INCREMENT_COUNT 100000

pthread_mutex_t mutex;
int shared_counter = 0;

static void *increment_counter(void *arg) {
    int thread_id = *(int *)arg;
    for (int i = 0; i < INCREMENT_COUNT; i++) {
        pthread_mutex_lock(&mutex);

        shared_counter++;

        pthread_mutex_unlock(&mutex);
    }
    printf("Thread %d finished.\n", thread_id);
    return NULL;
}

int main()
{
	pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    int ret;

    ret = pthread_mutex_init(&mutex, NULL);
    if (ret != 0) {
        perror("pthread_mutex_init");
        return -1;
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        ret = pthread_create(&threads[i], NULL, increment_counter, &thread_ids[i]);
        if (ret != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Final counter value: %d (expected: %d)\n", shared_counter, NUM_THREADS * INCREMENT_COUNT);

    pthread_mutex_destroy(&mutex);
}