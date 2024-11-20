#include <evl/thread.h>
#include <asm-generic/errno-base.h>
#include <evl/sched.h>
#include <bits/time.h>
#include <evl/mutex.h>
#include <evl/clock.h>
#include <evl/thread.h>
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
#include <epos/pthread.h>
#include <epos/wrappers.h>
#include <errno.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "internal.h"
#include <evl/mutex.h>

#define EVL_RANDOM_THREAD_NAME_LEGNTH 15
static pthread_attr_ex_t default_attr_ex;

struct pthread_iargs {
	sem_t sync;
	void *(*start)(void *);
	void *arg;
	int parent_prio;
	int ret;
	int flags;

	struct sched_param_ex param_ex;
	int policy;
	const char *name;
};

#define THREAD_TO_EFD_TABLE_MAX_SLOTS_SIZE (128)
#define THREAD_TO_EFD_TABLE_MAX_SLOTS_SIZE_IN_WORD                             \
	(THREAD_TO_EFD_TABLE_MAX_SLOTS_SIZE / sizeof(unsigned long))
#define THREAD_TO_EFD_TABLE_SIZE (THREAD_TO_EFD_TABLE_MAX_SLOTS_SIZE / 2)

struct hash_head {
	pthread_t key;
	int value;
	int next;
};

struct hash_table {
	struct hash_head slots[THREAD_TO_EFD_TABLE_MAX_SLOTS_SIZE];
	unsigned long
		available_slots[THREAD_TO_EFD_TABLE_MAX_SLOTS_SIZE_IN_WORD];
	int head[THREAD_TO_EFD_TABLE_SIZE];
	struct evl_mutex mutex; // can use rwlock instead
} thread_to_efd_table;

static void hash_table_init(struct hash_table *table);
static int get_available_slot(struct hash_table *table);
static void release_slot(struct hash_table *table, int slot);
static int hash_table_insert(struct hash_table *table, pthread_t key,
			     int value);
static int hash_table_lookup(struct hash_table *table, pthread_t key,
			     int *value);
static int hash_table_delete(struct hash_table *table, pthread_t key);

static void *evl_thread_trampoline(void *iarg)
{
	int parent_prio, efd, flags;
	void *(*start)(void *), *arg, *retval;
	char name_buf[EVL_RANDOM_THREAD_NAME_LEGNTH + 1];
	struct pthread_iargs *iargs = (struct pthread_iargs *)(iarg);
	volatile pthread_t ptid = pthread_self();
	struct evl_sched_attrs evl_sched_attr;
	long ret = 0;

	start = iargs->start;
	arg = iargs->arg;
	parent_prio = iargs->parent_prio;
	flags = iargs->flags;
	
	// get a random name
	memset(name_buf, 0, sizeof(name_buf));
	rand_str((char *)&name_buf, EVL_RANDOM_THREAD_NAME_LEGNTH);
	name_buf[EVL_RANDOM_THREAD_NAME_LEGNTH] = 0;
	if (iargs->name){
		efd = evl_attach_thread(flags, "%s",iargs->name);
	}else{
		efd = evl_attach_thread(flags, "%s-%s",evl_program_basename,name_buf);
	}
	if (efd < 0){
		ret = efd;
		goto sync_with_creator;
	}

	evl_sched_attr.sched_policy = iargs->policy;
	memcpy(&evl_sched_attr.sched_u, &iargs->param_ex.sched_u,
	       sizeof(evl_sched_attr.sched_u));
	evl_sched_attr.sched_priority = iargs->param_ex.sched_priority;

	ret = evl_set_schedattr(efd, &evl_sched_attr);
	if (ret){
		printf("sched_policy=%d prio=%d\n",evl_sched_attr.sched_policy,evl_sched_attr.sched_priority);
		goto sync_with_creator;
	}

	hash_table_insert(&thread_to_efd_table, ptid, efd);

sync_with_creator:
	iargs->ret = ret;
	__STD(sem_post(&iargs->sync));

	if (ret < 0)
		return (void *)ret;

	if (iargs->param_ex.sched_priority == parent_prio)
		__STD(sched_yield());

	evl_thread_harden();

	retval = start(arg);

	hash_table_delete(&thread_to_efd_table, ptid);
	return retval;
}

EPOS_IMPL(int, pthread_create,
	  (pthread_t * ptid_r, const pthread_attr_t *attr,
	   void *(*start)(void *), void *arg))
{
	pthread_attr_ex_t attr_ex;
	int policy;

	if (attr == NULL)
		attr = &default_attr_ex.std;

	memcpy(&attr_ex.std, attr, sizeof(*attr));
	pthread_attr_getschedpolicy(attr, &policy);
	attr_ex.nonstd.sched_policy = policy;
	attr_ex.nonstd.sched_param.sched_priority = default_attr_ex.nonstd.sched_param.sched_priority;
	attr_ex.nonstd.flags = default_attr_ex.nonstd.flags;
	attr_ex.nonstd.thread_name = NULL;

	return pthread_create_ex(ptid_r, &attr_ex, start, arg);
}

int pthread_create_ex(pthread_t *ptid_r, const pthread_attr_ex_t *attr_ex,
		      void *(*start)(void *), void *arg)
{
	struct pthread_iargs iargs;
	pthread_attr_t attr;
	int ret, inherit;
	struct timespec timeout;
	if (attr_ex == NULL)
		attr_ex = &default_attr_ex;

	pthread_getschedparam_ex(pthread_self(), &iargs.policy,
				 &iargs.param_ex);
	iargs.parent_prio = iargs.param_ex.sched_priority;
	memcpy(&attr, &attr_ex->std, sizeof(attr));

	pthread_attr_getinheritsched(&attr, &inherit);
	if (inherit == PTHREAD_EXPLICIT_SCHED) {
		pthread_attr_getschedpolicy_ex(attr_ex, &iargs.policy);
	}

	/*
	* Get the created thread to temporarily inherit the
	* caller priority (we mean linux/libc priority here,
	* as we use a libc call to create the thread).
	*/
	pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
	memcpy(&iargs.param_ex,&attr_ex->nonstd.sched_param,sizeof(attr_ex->nonstd.sched_param));

	iargs.start = start;
	iargs.arg = arg;
	iargs.ret = EAGAIN;
	iargs.name = attr_ex->nonstd.thread_name;
	iargs.flags = attr_ex->nonstd.flags;

	ret = __STD(sem_init(&iargs.sync, 0, 0));
	if (ret)
		goto create_thread_failed;

	ret = __STD(pthread_create)(ptid_r, &attr, evl_thread_trampoline,
				    &iargs);
	if (ret)
		goto create_thread_failed;

	__STD(clock_gettime(CLOCK_REALTIME, &timeout));
	timeout.tv_sec += 5;
	timeout.tv_nsec = 0;

	for (;;) {
		ret = __STD(sem_timedwait(&iargs.sync, &timeout));
		if (ret && errno == EINTR)
			continue;
		if (ret == 0) {
			if (iargs.ret < 0) {
				ret = iargs.ret;
			}
			break;
		} else if (errno == ETIMEDOUT) {
			ret = EAGAIN;
			break;
		}
		ret = -errno;
		printf("regular sem_wait failed with %s", symerror(ret));
		exit(1);
	}

	__STD(sem_destroy(&iargs.sync));

	evl_thread_harden();
	return ret;

create_thread_failed:
	__STD(sem_destroy(&iargs.sync));
	return ret;
}

EPOS_IMPL(int, pthread_join, (pthread_t thread, void **retval))
{
	return __STD(pthread_join(thread, retval));
}

EPOS_IMPL(int, pthread_yield, (void))
{
	return evl_yield();
}

// EPOS_IMPL(int, pthread_kill, (pthread_t ptid, int sig))
// {
// 	return 0;
// }

EPOS_IMPL(int, pthread_setschedprio, (pthread_t thread, int prio))
{
	struct evl_sched_attrs attr;
	int ret, efd;
	ret = get_evl_thread_fd(thread);
	if (ret < 0) {
		return ret;
	}
	efd = ret;
	ret = evl_get_schedattr(efd, &attr);

	if (ret < 0) {
		return ret;
	}

	// FIXME: maybe we should check the range of prio
	attr.sched_priority = prio;
	ret = evl_set_schedattr(efd, &attr);
	return ret;
}

EPOS_IMPL(int, pthread_setschedparam,
	  (pthread_t thread, int policy, const struct sched_param *param))
{
	struct evl_sched_attrs attr;
	int ret, efd;
	ret = get_evl_thread_fd(thread);
	if (ret < 0) {
		return ret;
	}
	efd = ret;

	ret = evl_get_schedattr(efd, &attr);
	if (ret < 0) {
		return ret;
	}

	attr.sched_policy = policy;

	if (param) {
		attr.sched_priority = param->sched_priority;
	}

	ret = evl_set_schedattr(efd, &attr);
	return ret;
}

EPOS_IMPL(int, pthread_getschedparam,
	  (pthread_t thread, int *policy, struct sched_param *param))
{
	struct evl_sched_attrs attr;
	int ret, efd;
	ret = get_evl_thread_fd(thread);
	if (ret < 0) {
		return ret;
	}
	efd = ret;

	ret = evl_get_schedattr(efd, &attr);
	if (ret < 0) {
		return ret;
	}
	if (param) {
		param->sched_priority = attr.sched_priority;
	}
	if (policy) {
		*policy = attr.sched_policy;
	}
	return 0;
};

int pthread_getschedparam_ex(pthread_t ptid, int *policy,
			     struct sched_param_ex *param)
{
	struct evl_sched_attrs attr;
	int ret, efd;
	ret = get_evl_thread_fd(ptid);
	if (ret < 0) {
		return ret;
	}
	efd = ret;

	ret = evl_get_schedattr(efd, &attr);

	if (policy) {
		*policy = attr.sched_policy;
	}

	if (param) {
		param->sched_priority = attr.sched_priority;
		memcpy(&param->sched_u, &attr.sched_u, sizeof(param->sched_u));
	}
	return 0;
}

int pthread_setschedparam_ex(pthread_t ptid, int policy,
			     const struct sched_param_ex *param)
{
	struct evl_sched_attrs attr;
	int ret, efd;
	if (!param) {
		return -EINVAL;
	}

	ret = get_evl_thread_fd(ptid);
	if (ret < 0) {
		return ret;
	}
	efd = ret;

	ret = evl_get_schedattr(efd, &attr);

	if (ret < 0) {
		return ret;
	}

	attr.sched_policy = policy;
	memcpy(&attr.sched_u, &param->sched_u, sizeof(attr.sched_u));
	attr.sched_priority = param->sched_priority;

	ret = evl_set_schedattr(efd, &attr);
	return ret;
}

static int hash_func(pthread_t key)
{
	return key % THREAD_TO_EFD_TABLE_SIZE;
}

static void hash_table_init(struct hash_table *table)
{
	memset(table, 0, sizeof(struct hash_table));
	for (int i = 0; i < THREAD_TO_EFD_TABLE_SIZE; ++i) {
		table->head[i] = -1;
	}
	for (int i = 0; i < THREAD_TO_EFD_TABLE_MAX_SLOTS_SIZE_IN_WORD; ++i) {
		table->available_slots[i] = (unsigned long)-1;
	}
	evl_new_mutex(&table->mutex, "_thread_to_fd_hash_table_mutex");
}

// locked
static int get_available_slot(struct hash_table *table)
{
	for (int i = 0; i < THREAD_TO_EFD_TABLE_MAX_SLOTS_SIZE_IN_WORD; ++i) {
		if (table->available_slots[i] != 0) {
			int bit_position =
				__builtin_ctzl(table->available_slots[i]);
			table->available_slots[i] &= ~(1L << bit_position);
			return i * LONG_BIT + bit_position;
		}
	}
	return -1;
}

// locked
static void release_slot(struct hash_table *table, int slot)
{
	int word = slot / (sizeof(long) * 8);
	int bit = slot % (sizeof(long) * 8);
	table->available_slots[word] |= (1L << bit);
}

static int hash_table_insert(struct hash_table *table, pthread_t key, int value)
{
	int hash_idx = hash_func(key);

	evl_lock_mutex(&table->mutex);
	int slot = get_available_slot(table);
	if (slot == -1) {
		evl_unlock_mutex(&table->mutex);
		return -1;
	}

	table->slots[slot].key = key;
	table->slots[slot].value = value;
	table->slots[slot].next = table->head[hash_idx];
	table->head[hash_idx] = slot;
	evl_unlock_mutex(&table->mutex);
	return 0;
}

static int hash_table_lookup(struct hash_table *table, pthread_t key,
			     int *value)
{
	int hash_idx = hash_func(key);
	evl_lock_mutex(&table->mutex);
	int slot = table->head[hash_idx];
	while (slot != -1) {
		if (table->slots[slot].key == key) {
			*value = table->slots[slot].value;
			evl_unlock_mutex(&table->mutex);
			return 0;
		}
		slot = table->slots[slot].next;
	}
	evl_unlock_mutex(&table->mutex);
	return -1;
}

static int hash_table_delete(struct hash_table *table, pthread_t key)
{
	int hash_idx = hash_func(key);
	evl_lock_mutex(&table->mutex);
	int prev_slot;
	int slot = table->head[hash_idx];
	prev_slot = -1;
	while (slot != -1) {
		if (table->slots[slot].key == key) {
			if (prev_slot != -1) {
				table->slots[prev_slot].next =
					table->slots[slot].next;
			} else {
				table->head[hash_idx] = table->slots[slot].next;
				;
			}
			release_slot(table, slot);
			evl_unlock_mutex(&table->mutex);
			return 0;
		}
		prev_slot = slot;
		slot = table->slots[slot].next;
	}
	evl_unlock_mutex(&table->mutex);
	return -1;
}

int get_evl_thread_fd(pthread_t thread)
{
	int value, ret;
	ret = hash_table_lookup(&thread_to_efd_table, thread, &value);
	if (ret != -1) {
		return value;
	} else {
		return -1;
	}
}

void epos_thread_init(void)
{
	pthread_t tid;
	pthread_attr_init_ex(&default_attr_ex);
	hash_table_init(&thread_to_efd_table);
	tid = pthread_self();
	hash_table_insert(&thread_to_efd_table, tid, main_thread_efd);
}