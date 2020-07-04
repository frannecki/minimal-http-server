#include "threadpool.h"


int threadpool_create(struct threadpool_t* pool, int maxnum){
    int i;
    thread_task_init(&(pool->tasks));
    pool->stopped = 0;
    pool->maxnum = maxnum;
    pool->threads = (pthread_t*)malloc(pool->maxnum * sizeof(pthread_t));
    if(pthread_cond_init(&(pool->cond), NULL)){
        return THREAD_ERROR;
    }
    if(pthread_mutex_init(&(pool->mtx), NULL)){
        return THREAD_ERROR;
    }
    for(i = 0; i < pool->maxnum; ++i){
        pthread_create(&(pool->threads[i]), NULL, &threadpool_dowork, (void*)pool);
    }
    return THREAD_CREATED;
}


static void* threadpool_dowork(void* param){
    struct threadpool_t* pool = (struct threadpool_t*)param;
    thread_fun_t task;
    void* args;
    unsigned char newtask;
    for(;;){
        pthread_mutex_lock(&(pool->mtx));
        if(pool->stopped){
            break;
        }
        while(!pool->stopped && pool->tasks.task_cnt <= 0){
            pthread_cond_wait(&(pool->cond), &(pool->mtx));
        }
        newtask = 0;
        if(pool->tasks.task_cnt > 0){
            task = pool->tasks.tasks[pool->tasks.front].func;
            args = pool->tasks.tasks[pool->tasks.front].args;
            thread_task_pop(&(pool->tasks));
            newtask = 1;
        }
        pthread_mutex_unlock(&(pool->mtx));
        if(newtask)  (*task)(args);
    }
    pthread_mutex_unlock(&(pool->mtx));
}


int threadpool_assign_task(struct threadpool_t* pool, thread_fun_t task, void* args){
    pthread_mutex_lock(&(pool->mtx));
    if(pool->stopped){
        return THREAD_FINISHED;
    }
    int back = (pool->tasks.front + pool->tasks.task_cnt) % pool->tasks.max_task_cnt;
    pool->tasks.tasks[back].func = task;
    pool->tasks.tasks[back].args = args;
    pool->tasks.task_cnt++;
    if(++back >= pool->tasks.max_task_cnt){
        back = 0;
    }
    if(pool->tasks.task_cnt * 2 >= pool->tasks.max_task_cnt){
        thread_task_double(&(pool->tasks));
    }
    pthread_mutex_unlock(&(pool->mtx));
    pthread_cond_signal(&(pool->cond));
    return THREAD_TASK;
}


int threadpool_stop(struct threadpool_t* pool){
    int i = 0;
    void* res;
    pthread_mutex_lock(&(pool->mtx));
    pool->stopped = 1;
    pthread_mutex_unlock(&(pool->mtx));
    pthread_cond_broadcast(&(pool->cond));
    for(i = 0; i < pool->maxnum; ++i){
        pthread_join(pool->threads[i], &res);
    }
    free(pool->threads);
    free(pool->tasks.tasks);
    
    if(pthread_mutex_destroy(&(pool->mtx))){
        return THREAD_ERROR;
    }
    if(pthread_cond_destroy(&(pool->cond))){
        return THREAD_ERROR;
    }
    return THREAD_FINISHED;
}


static void thread_task_init(struct thread_task_t* tasks){
    tasks->task_cnt = 0;
    tasks->front = 0;
    tasks->max_task_cnt = 20;
    tasks->tasks = (thread_fun_var_t*)malloc(tasks->max_task_cnt * sizeof(thread_fun_var_t));
}


static void thread_task_pop(struct thread_task_t* tasks){
    if(++tasks->front >= tasks->max_task_cnt){
        tasks->front = 0;
    }
    tasks->task_cnt--;
}


static void thread_task_double(struct thread_task_t* t){
    int max_task_cnt = 2 * t->max_task_cnt;
    int leftover = 0;
    int back = 0;
    if((back = t->front + t->task_cnt) > t->max_task_cnt){
        leftover = back % t->max_task_cnt;
    }
    thread_fun_var_t* tasks = (thread_fun_var_t*)malloc(max_task_cnt * sizeof(thread_fun_var_t));
    memcpy(tasks, t->tasks + t->front, (t->task_cnt - leftover) * sizeof(thread_fun_var_t));
    memcpy(tasks + t->task_cnt - leftover, t->tasks, leftover * sizeof(thread_fun_var_t));
    t->tasks = tasks;
    t->front = 0;
    t->max_task_cnt = max_task_cnt;
}