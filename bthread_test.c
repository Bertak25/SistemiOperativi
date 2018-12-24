
// Created by edoardo on 22/10/18.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "bthread.h"

void *function1(void *arg) {
    bthread_printf("Sono il thread 1\n");
}

void *function2(void *arg) {
    bthread_testcancel();
    bthread_printf("Dormo per 2 secondi \n");
    bthread_sleep(2000);
    bthread_printf("Sono il thread 2, ho aspettato 2 secondi.\n");
}

void *function3(void *arg) {
    bthread_testcancel();
    bthread_printf("Sono il thread 3\n");
}

int main() {
//    bthread_set_schedulong_routine(bthread_round_robin_scheduling);
    bthread_set_schedulong_routine(bthread_priority_scheduling);
//    bthread_set_schedulong_routine(bthread_random_scheduling());
//    bthread_set_schedulong_routine(bthread_lottery_scheduling);

    bthread_t bthread1, bthread2, bthread3;

    bthread_create(&bthread1, NULL, function1, NULL);
    bthread_create(&bthread2, NULL, function2, NULL);
    bthread_create(&bthread3, NULL, function3, NULL);


    printf("t1 %lu | t2 %lu | t3 %lu \n", bthread1, bthread2, bthread3);

    // richiedo la cancellazione del thread 3, se controlla si suicida.
    //bthread_cancel(bthread2);

    bthread_join(bthread1, NULL);
    printf("join thread 0\n");
    bthread_join(bthread2, NULL);
    printf("join thread 1\n");
    bthread_join(bthread3, NULL);
    printf("join thread 2\n");

    printf("Exiting main. \n");
}
