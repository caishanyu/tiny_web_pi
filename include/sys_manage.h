#ifndef __SYS_MANAGE_H__
#define __SYS_MANAGE_H__

#include "debug.h"
#include "common_def.h"

#define MEMINFO_DIR ("/proc/meminfo")

/*
    Name:   sys_manage_init
    Func:   initialize sys_manage module
    In:
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code sys_manage_init();

/*
    Name:   sys_manage_get_meminfo
    In:
    Out:    meminfo
    Return:
            0 - ok
            others - fail
*/
error_code sys_manage_get_meminfo(OUT char *meminfo);

#endif