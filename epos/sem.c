#include <assert.h>
#include <evl/sem.h>
#include <epos/semaphore.h>
#include "internal.h"
#include <errno.h>
#include <semaphore.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define EVL_RANDOM_SEM_NAME_LEGNTH 15
#define EVL_MAX_SEM_NAME_BUFFER 256

static_assert(sizeof(sem_t) >= sizeof(struct evl_sem),
	      "size of sem_t must greater than evl_sem");

struct evl_sem *get_evl_sem(sem_t *std_sem)
{
	return (struct evl_sem *)std_sem;
}

EPOS_IMPL(int, sem_init, (sem_t * std_sem, int pshared, unsigned int value))
{
	char name_buf[EVL_RANDOM_SEM_NAME_LEGNTH + 1];
	struct evl_sem *sem = get_evl_sem(std_sem);
	int ret;
	if (pshared == true) {
		// try use `sem_open` instead.
		return -EINVAL;
	}

	rand_str((char *)&name_buf, EVL_RANDOM_SEM_NAME_LEGNTH);
	ret = evl_create_sem(sem, EVL_CLOCK_MONOTONIC, value, EVL_CLONE_PRIVATE,
			     "%s-sem-%s", evl_program_basename, name_buf);

	return ret;
}

EPOS_IMPL(int, sem_destroy, (sem_t * std_sem))
{
	struct evl_sem *sem = get_evl_sem(std_sem);
    evl_close_sem(sem);
}

EPOS_IMPL(int, sem_post, (sem_t * std_sem))
{
	struct evl_sem *sem = get_evl_sem(std_sem);
	return evl_put_sem(sem);
}

EPOS_IMPL(int, sem_wait, (sem_t * std_sem))
{
	struct evl_sem *sem = get_evl_sem(std_sem);
	return evl_get_sem(sem);
}

EPOS_IMPL(int, sem_timedwait, (sem_t * std_sem, const struct timespec *abs_timeout))
{
	struct evl_sem *sem = get_evl_sem(std_sem);
	return evl_timedget_sem(sem,abs_timeout);
}

EPOS_IMPL(int, sem_trywait, (sem_t * std_sem)){
    struct evl_sem *sem = get_evl_sem(std_sem);
	return evl_tryget_sem(sem);
}

EPOS_IMPL(int, sem_getvalue, (sem_t * std_sem, int *value)){
    struct evl_sem *sem = get_evl_sem(std_sem);
	return evl_peek_sem(sem,value);
}

EPOS_IMPL(sem_t *, sem_open, (const char *name, int oflags, ...)){
    // TODO: 使用malloc
    printf("Not support regular sem_open. Try sem_open_np instead!");
	exit(1);
}

EPOS_IMPL(int, sem_close, (sem_t * std_sem)){
	struct evl_sem *sem = get_evl_sem(std_sem);
    evl_close_sem(sem);
}

EPOS_IMPL(int, sem_unlink, (const char *name)){
    return -EINVAL;
}

int sem_open_np(sem_t *std_sem,const char *name, int oflags, ...){
    char name_buf[EVL_MAX_SEM_NAME_BUFFER];
    struct evl_sem *sem;
    int ret;
    if (name && name[0] == '/'){
        return -EINVAL;
    }

    ret = snprintf(name_buf, EVL_MAX_SEM_NAME_BUFFER-1, "/dev/evl/sem/%s",name);
    if (ret)
        return ret;

   sem = get_evl_sem(std_sem);
    if (access(name_buf, F_OK)){
        if (oflags & (O_CREAT | O_EXCL)){
            return -EEXIST;
        }
        return evl_open_sem(sem,name);
    }else{
        // evl_open_sem(struct evl_sem *sem, const char *fmt, ...)
        if (!(oflags & O_CREAT)){
            return -ENOENT;
        }
        return evl_create_sem(sem, EVL_CLOCK_MONOTONIC, 0, EVL_CLONE_PUBLIC, name);
    }
}