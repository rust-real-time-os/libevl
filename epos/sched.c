#include "epos_internal.h"
#include <epos/sched.h>
#include <evl/sched.h>

EPOS_IMPL(int, sched_yield, (void))
{
    return evl_yield();
}
