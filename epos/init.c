#include <evl/thread.h>
#include "internal.h"

static void epos_init(void)
{							
	evl_attach_self("main");
}

int epos_main(int argc, char *const argv[]);

int __real_main(int argc, char *const argv[]);

int __wrap_main(int argc, char *const argv[])
__attribute__((alias("epos_main"), weak));


int epos_main(int argc, char *const argv[])
{
	evl_program_basename = get_process_name(argv[0]);
	epos_init();
	return __real_main(argc, argv);
}