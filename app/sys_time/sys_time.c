#include <string.h>
#include <pthread.h>
#include "sys_time.h"
#include "common_def.h"
#include "http.h"

static pthread_mutex_t sys_time_mtx = PTHREAD_MUTEX_INITIALIZER;

#define ST_LOCK         pthread_mutex_lock(&sys_time_mtx);
#define ST_UNLOCK       pthread_mutex_unlock(&sys_time_mtx);

static error_code sys_time_gen_html(OUT char *buffer);

/*
    Name:   sys_time_init
    Func:   initialize sys_time module
    In:
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code sys_time_init()
{
    PFM_IF_FAIL_RET(http_gen_func_register(sys_time_gen_html));

    DBG_MAJ("sys_time module init OK");

    return err_no_error;
}

/*
    Name:   sys_time_gen_html
    Func:   generate info of sys_time, display in web
    In:
    Out:    buffer      - sysTime info for html
    Return:
            0 - ok
            others - fail
*/
static error_code sys_time_gen_html(OUT char *buffer)
{
    char sys_time[SYS_TIME_STR_MAX_LEN] = {};

    PFM_ENSURE_RET_VAL(buffer, err_bad_param);

    PFM_IF_FAIL_RET(sys_time_get_str(SYS_TIME_STR_MAX_LEN, sys_time));

    strcat(buffer, HEADER_1 "system time" HEADER_1_END);
    strcat(buffer, PARAGRAPH);
    strcat(buffer, sys_time);
    strcat(buffer, PARAGRAPH_END);

    return err_no_error;
}

/*
    Name:   sys_time_get_str
    Func:   get current time from sys(linux), return string format
    In:     buffer_size     - sizeof buffer
    Out:    buffer          - string format time
    Return:
            0 - ok
            others - fail
*/
error_code sys_time_get_str(IN int buffer_size, OUT char *buffer)
{
    time_t rawtime = 0;
    struct tm *timeinfo = NULL;

    PFM_ENSURE_RET_VAL(buffer_size >= SYS_TIME_STR_MAX_LEN, err_bad_param);

    ST_LOCK;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    memset(buffer, 0, buffer_size);
    strftime(buffer, SYS_TIME_STR_MAX_LEN, "%Y-%m-%d %H:%M:%S", timeinfo);

    ST_UNLOCK;

    return err_no_error;
}