#ifndef THREADPOOL_H
#define THREADPOOL_H
#ifdef __cplusplus
extern "C"{
#endif
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#ifdef __cplusplus
}
#endif

enum THREADSTATUS{
    THREAD_CREATED = 0,
    THREAD_TASK,
    THREAD_STOPPED,
    THREAD_FINISHED,
    THREAD_ERROR
};

//typedef void(*thread_fun_t)(void);
typedef int(*thread_fun_t)(void*);

typedef struct thread_fun_var_t{
    thread_fun_t func; 
    void* args;
}thread_fun_var_t;

struct thread_task_t{
    int task_cnt;
    int max_task_cnt;
    int front;
    thread_fun_var_t* tasks;
    //pthread_mutex_t mtx_task;
};

struct threadpool_t{
    int maxnum;  
    unsigned int stopped;
    pthread_mutex_t mtx;
    pthread_cond_t cond;
    pthread_t* threads;
    struct thread_task_t tasks;
};

int threadpool_create(struct threadpool_t*, int);

static void* threadpool_dowork(void*);

int threadpool_assign_task(struct threadpool_t*, thread_fun_t, void*);

int threadpool_stop(struct threadpool_t*);

static void thread_task_init(struct thread_task_t*);

static void thread_task_pop(struct thread_task_t*);

static void thread_task_double(struct thread_task_t*);

#endif