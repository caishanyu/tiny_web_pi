#include <string.h>
#include <stdlib.h>
#include "thread_pool.h"

static void* thread_worker(void *args)
{
    struct thread_pool_s *p_pool = (struct thread_pool_s*)args;
    struct task_s *p_task = NULL;

    while(1)
    {
        pthread_mutex_lock(&p_pool->thread_pool_mtx);

        // wait task
        while(p_pool->running && NULL == p_pool->task_queue_head.next)
        {
            pthread_cond_wait(&p_pool->task_cond, &p_pool->thread_pool_mtx);
        }

        // thread pool is destroying
        if(!p_pool->running)
        {
            pthread_mutex_unlock(&p_pool->thread_pool_mtx);
            pthread_exit(NULL);
        }

        // get a task from task queue header
        p_task = p_pool->task_queue_head.next;
        p_pool->task_queue_head.next = p_task->next;

        pthread_mutex_unlock(&p_pool->thread_pool_mtx);

        // execute task
        p_task->func(p_task->args);
        free(p_task);
        DBG("thread tid %lu execute task done", pthread_self());
    }

    return NULL;
}

/*
    Name:   thread_pool_create
    Func:   create a thread pool
    In:     thread_count    - counts of threads in pool, 1~MAX_THREAD_COUNT
    Out:    p_thread_pool   - pointer to thread_pool
    Return:
            0 - ok
            others - fail
*/
error_code thread_pool_create(IN int thread_count, OUT struct thread_pool_s *p_thread_pool)
{
    error_code ret = err_no_error;
    int i = 0;
    int j = 0;

    PFM_ENSURE_RET_VAL(thread_count < MAX_THREAD_COUNT && thread_count > 0 &&  p_thread_pool, err_bad_param);

    memset(p_thread_pool, 0, sizeof(struct thread_pool_s));

    pthread_cond_init(&p_thread_pool->task_cond, NULL);
    pthread_mutex_init(&p_thread_pool->thread_pool_mtx, NULL);
    p_thread_pool->running = 1;

    // malloc space of threads array
    p_thread_pool->thread_array = (pthread_t*)malloc(sizeof(pthread_t)*thread_count);
    if(!p_thread_pool->thread_array)
    {
        ret = err_no_memory;
        goto error;
    }
    memset(p_thread_pool->thread_array, 0, sizeof(pthread_t)*thread_count);
    // create threads
    for(i = 0; i < thread_count; ++ i)
    {
        if(pthread_create(&(p_thread_pool->thread_array[i]), NULL, thread_worker, (void*)p_thread_pool) < 0)
        {
            ret = err_pthread_opera_fail;
            goto error;
        }
    }
    p_thread_pool->thread_count = thread_count;

    DBG("thread pool create done");

    return err_no_error;

error:
    if(i != thread_count)       // handle 
    {
        for(j = 0; j < i; ++ j)
        {
            pthread_cancel(p_thread_pool->thread_array[j]);
            pthread_join(p_thread_pool->thread_array[j], NULL);
        }
    }
    pthread_mutex_destroy(&p_thread_pool->thread_pool_mtx);
    pthread_cond_destroy(&p_thread_pool->task_cond);
    if(p_thread_pool->thread_array)
        free(p_thread_pool->thread_array);
    memset(p_thread_pool, 0, sizeof(struct thread_pool_s));
    
    DBG_ERR("create thread_pool fail, ret %d", ret);
    return ret;
}

/*
    Name:   thread_pool_destroy
    Func:   destroy a thread pool
    In:     p_thread_pool   - pointer to thread_pool
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code thread_pool_destroy(IN struct thread_pool_s *p_thread_pool)
{
    int i = 0;

    PFM_ENSURE_RET_VAL(p_thread_pool, err_bad_param);

    pthread_mutex_lock(&p_thread_pool->thread_pool_mtx);
    if(!p_thread_pool->running)
    {
        pthread_mutex_unlock(&p_thread_pool->thread_pool_mtx);
        return err_thread_pool_not_init;
    }
    else
    {
        p_thread_pool->running = 0;     // set shutdown flag
        pthread_cond_broadcast(&p_thread_pool->task_cond);   // wakeup all threads
    }
    pthread_mutex_unlock(&p_thread_pool->thread_pool_mtx);

    // wait all threads quit
    for(i = 0; i < p_thread_pool->thread_count; ++ i)
    {
        pthread_join(p_thread_pool->thread_array[i], NULL);
    }

    DBG("thread pool destroy done");

    return err_no_error;
}

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
error_code thread_pool_submit_task(IN struct thread_pool_s *p_thread_pool, IN task_func func, IN void *args)
{
    struct task_s *p_task = NULL;
    struct task_s *ptr = NULL;

    p_task = (struct task_s*)malloc(sizeof(struct task_s));
    if(!p_task)
    {
        return err_no_memory;
    }
    p_task->func = func;
    p_task->args = args;
    p_task->next = NULL;

    PFM_ENSURE_RET_VAL(p_thread_pool && func, err_bad_param);

    pthread_mutex_lock(&p_thread_pool->thread_pool_mtx);

    if(!p_thread_pool->running)
    {
        pthread_mutex_unlock(&p_thread_pool->thread_pool_mtx);
        free(p_task);
        return err_thread_pool_not_init;
    }

    ptr = &p_thread_pool->task_queue_head;
    while(ptr->next)
    {
        ptr = ptr->next;
    }
    ptr->next = p_task;

    pthread_cond_signal(&p_thread_pool->task_cond);
    pthread_mutex_unlock(&p_thread_pool->thread_pool_mtx);

    return err_no_error;
}