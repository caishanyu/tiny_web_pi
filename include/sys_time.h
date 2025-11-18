#ifndef __SYS_TIME_H__
#define __SYS_TIME_H__

#include <time.h>
#include "debug.h"

#define SYS_TIME_STR_MAX_LEN    (32)    // "%Y-%m-%d %H:%M:%S" "2000-12-30 23:59:59" len 19

/*
    Name:   sys_time_init
    Func:   initialize sys_time module
    In:
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code sys_time_init();

/*
    Name:   sys_time_get_str
    Func:   get current time from sys(linux), return string format
    In:     buffer_size     - sizeof buffer
    Out:    buffer          - string format time
    Return:
            0 - ok
            others - fail
*/
error_code sys_time_get_str(IN int buffer_size, OUT char *buffer);

#endif