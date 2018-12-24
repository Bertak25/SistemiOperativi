//
// Created by edoardo on 18/10/18.
//
#include <stdlib.h>
#include <stdarg.h>

#ifndef BTHREAD_BTHREAD_H
#define BTHREAD_BTHREAD_H

typedef unsigned long int bthread_t;

typedef enum {
    __BTHREAD_READY = 0, __BTHREAD_BLOCKED, __BTHREAD_SLEEPING,
    __BTHREAD_ZOMBIE
} bthread_state;

typedef struct {
} bthread_attr_t;

typedef void *(*bthread_routine)(void *);

typedef void (*bthread_scheduling_routine)();

int bthread_create(bthread_t *bthread, const bthread_attr_t *attr, void *(*start_routine)(void *), void *arg);

int bthread_join(bthread_t bthread, void **retval);

void bthread_yield();

void bthread_exit(void *retval);

int bthread_cancel(bthread_t bthread);

void bthread_testcancel(void);

void bthread_sleep(double ms);

void bthread_printf(const char* format, ...);

void bthread_set_schedulong_routine(void *(*bthread_scheduling_routine)(void *));

void *bthread_round_robin_scheduling();

void *bthread_priority_scheduling();

void *bthread_random_scheduling();

void *bthread_lottery_scheduling();

#endif //BTHREAD_BTHREAD_H
