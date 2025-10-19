#include <stdarg.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include "debug.h"
#include "sys_time.h"

pthread_mutex_t debug_mtx = PTHREAD_MUTEX_INITIALIZER;

void dbg(dbg_level lev, const char *func, int line, char *fmt, ...)
{
    va_list list;
    FILE *fp = NULL;
    char sys_time[SYS_TIME_STR_MAX_LEN] = {};

    pthread_mutex_lock(&debug_mtx);

    va_start(list, fmt);

    fp = fopen(LOG_DIR, "a");
    if(!fp)
    {
        goto terminal;
    }
    sys_time_get_str(SYS_TIME_STR_MAX_LEN, sys_time);
    fprintf(fp, "[%s] <%s-%d> ", sys_time, func, line);
    vfprintf(fp, fmt, list);
    fprintf(fp, "\r\n");
    va_end(list);
    fclose(fp);

terminal:

#ifdef DBG_ON
    va_start(list, fmt);
    switch(lev)
    {
        case dbg_normal:
            printf("[%s] <%s-%d> ", sys_time, func, line);
            break;
        case dbg_major:
            printf(GREEN_CLR "[%s] <%s-%d> " RESET_CLR, sys_time, func, line);
            break;
        case dbg_error:
            printf(RED_CLR "[%s] <%s-%d> " RESET_CLR, sys_time, func, line);
            break;
    }
    vprintf(fmt, list);
    va_end(list);
    printf("\r\n");
#endif
    pthread_mutex_unlock(&debug_mtx);
}