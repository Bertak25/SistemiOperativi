//
// Created by edoardo on 18/10/18.
//

#ifndef BTHREAD_BTHREAD_PRIVATE_H
#define BTHREAD_BTHREAD_PRIVATE_H

#include <setjmp.h>
#include <signal.h>
#include "bthread.h"
#include "tqueue.h"

#define save_context(CONTEXT) sigsetjmp(CONTEXT, 1)
#define restore_context(CONTEXT) siglongjmp(CONTEXT, 1)


#define STACK_SIZE 64000
#define QUANTUM_USEC 100000



typedef struct {
    bthread_t tid;
    bthread_routine body;
    void *arg;
    bthread_state state;
    bthread_attr_t attr;
    char *stack;
    jmp_buf context;
    void *retval;
    double wake_up_time;
    int cancel_req;
    int priority;
    int quantumCounter;
} __bthread_private;

typedef struct {
    TQueue queue;
    TQueue current_item;
    jmp_buf context;
    bthread_t current_tid;
    bthread_scheduling_routine scheduling_routine;
} __bthread_scheduler_private;

__bthread_scheduler_private *bthread_get_scheduler();

void bthread_cleanup();

static void bthread_setup_timer();

static int bthread_check_if_zombie(bthread_t bthread, void **retval);

static TQueue bthread_get_queue_at(bthread_t bthread);

double get_current_time_millis();

void bthread_block_timer_signal();

void bthread_unblock_timer_signal();

#endif //BTHREAD_BTHREAD_PRIVATE_H
