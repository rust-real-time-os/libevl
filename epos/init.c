#include <evl/thread.h>
#include "epos_internal.h"
#include <stdio.h>

static void epos_init(const char* main)
{							
	evl_attach_thread(EVL_CLONE_PUBLIC, main);
	printf("I am run in evl\n");
}

int epos_main(int argc, char *const argv[]);

int __real_main(int argc, char *const argv[]);

int __wrap_main(int argc, char *const argv[])
__attribute__((alias("epos_main"), weak));

int epos_main(int argc, char *const argv[])
{
	evl_program_basename = argv[0];
	epos_init(evl_program_basename);
	return __real_main(argc, argv);
}