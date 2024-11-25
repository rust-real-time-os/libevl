#include <stdio.h>
#include <time.h>
#include <unistd.h>

static struct timespec timespec_diff(struct timespec start, struct timespec end) {
    struct timespec diff;

    diff.tv_sec = end.tv_sec - start.tv_sec;
    diff.tv_nsec = end.tv_nsec - start.tv_nsec;

    if (diff.tv_nsec < 0) {
        diff.tv_sec -= 1;         
        diff.tv_nsec += 1000000000; 
    }

    return diff;
}

int main(){
    struct timespec spec_start,spec_end,result;
    printf("sleep\n");
    clock_gettime(CLOCK_MONOTONIC,&spec_start);
    sleep(1);
    clock_gettime(CLOCK_MONOTONIC,&spec_end);
    result = timespec_diff(spec_start,spec_end);
    printf("sleep=%lds %ldus\n",result.tv_sec,result.tv_nsec / 1000);
    return 0;
}