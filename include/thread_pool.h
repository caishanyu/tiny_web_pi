#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <stdio.h>
#include <pthread.h>
#include "debug.h"

#define MAX_THREAD_COUNT            (32)

typedef void (*task_func)(void *args);

struct task_s
{
    task_func func;
    void *args;
    struct task_s *next;
};

struct thread_pool_s
{
    pthread_mutex_t thread_pool_mtx;    // mutex for pool
    
    pthread_t *thread_array;            // threads array
    int thread_count;

    struct task_s task_queue_head;      // header node of task queue
    pthread_cond_t task_cond;           // task queue cond

    int running;                       // running flag
};

/*
    Name:   thread_pool_create
    Func:   create a thread pool
    In:     thread_count    - counts of threads in pool, 1~MAX_THREAD_COUNT
    Out:    p_thread_pool   - pointer to thread_pool
    Return:
            0 - ok
            others - fail
*/
error_code thread_pool_create(IN int thread_count, OUT struct thread_pool_s *p_thread_pool);

/*
    Name:   thread_pool_destroy
    Func:   destroy a thread pool
    In:     p_thread_pool   - pointer to thread_pool
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code thread_pool_destroy(IN struct thread_pool_s *p_thread_pool);

/*
    Name:   thread_pool_submit_task
    Func:   submit a task into thread pool
    In:     p_thread_pool   - pointer to thread_pool
            func            - function
            args            - args
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code thread_pool_submit_task(IN struct thread_pool_s *p_thread_pool, IN task_func func, IN void *args);

#endif