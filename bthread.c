//
// Created by edoardo on 22/10/18.
//

#include <stdint.h>
#include <signal.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdbool.h>
#include "bthread_private.h"
#include "bthread.h"

// GLOBALS
static bthread_t tidCounter = 0;
static sigset_t *mask = NULL;


// STATIC

//Checks whether the thread referenced by the parameter bthread has reached a zombie state. If
//it's not the case the function returns 0. Otherwise the following steps are performed: if retval is
//not NULL the exit status of the target thread (i.e. the value that was supplied to bthread_exit)
//is copied into the location pointed to by *retval; the thread's stack is freed and the thread's
//private data structure is removed from the queue (Note: depending on your implementation, you
//might need to pay attention to the special case where the scheduler's queue pointer itself
//changes!); finally the function returns 1.
static int bthread_check_if_zombie(bthread_t bthread, void **retval) {
    volatile TQueue tQueue = bthread_get_queue_at(bthread);
    volatile __bthread_scheduler_private *scheduler = bthread_get_scheduler();
    volatile __bthread_private *thread = (__bthread_private *) (tqueue_get_data(tQueue));
    if (thread->state != __BTHREAD_ZOMBIE)
        return 0;
    else {
        if (retval != NULL) {
            *retval = thread->retval;
        }
        free(tqueue_pop(&tQueue));
        scheduler->queue = tQueue;
        return 1;
    }
}

//Returns a "view" on the queue beginning at the node containing data for the thread identified by
//bthread. If the queue is empty or doesn't contain the corresponding data this function returns NULL.
static TQueue bthread_get_queue_at(bthread_t bthread) {
    volatile __bthread_scheduler_private *scheduler = bthread_get_scheduler();
    for (int i = 0; i < tqueue_size(scheduler->queue); i++) {
        volatile __bthread_private *thread = (__bthread_private *) tqueue_at_offset(scheduler->queue, i)->data;
        if (thread->tid == bthread) {
            return tqueue_at_offset(scheduler->queue, i);
        }
    }
    return NULL;
}

//This private function creates, maintains and returns a static pointer to the singleton instance of
//__bthread_scheduler_private. Fields of this structure need to be initialized as NULL. Other functions
//will call this method to obtain this pointer. This function should not be accessible outside the library.
__bthread_scheduler_private *bthread_get_scheduler() {
    static __bthread_scheduler_private *scheduler_private = NULL; // come se fosse dichiarata all'esterno della funzione. è STATICA
    if (!scheduler_private) {
        scheduler_private = (__bthread_scheduler_private *) malloc(sizeof(__bthread_scheduler_private));
        scheduler_private->queue = NULL;
        scheduler_private->current_item = NULL;
        scheduler_private->current_tid = NULL;
    }
    return scheduler_private;
}

static void bthread_setup_timer() {
    static bool initialized = false;
    if (!initialized) {
        signal(SIGVTALRM, (void (*)()) bthread_yield);
        struct itimerval time;
        time.it_interval.tv_sec = 0;
        time.it_interval.tv_usec = QUANTUM_USEC;
        time.it_value.tv_sec = 0;
        time.it_value.tv_usec = QUANTUM_USEC;
        initialized = true;
        setitimer(ITIMER_VIRTUAL, &time, NULL);
    }
}

// THREAD

//Creates a new thread structure and puts it at the end of the queue. The thread identifier (stored
//in the buffer pointed by bthread) corresponds to the position in the queue. The thread is not
//started when calling this function. Attributes passed through the attr argument are ignored
//(thus it is possible to pass a NULL pointer). The stack pointer for new created threads is NULL.
int bthread_create(bthread_t *bthread, const bthread_attr_t *attr, void *(*start_routine)(void *), void *arg) {
    volatile __bthread_scheduler_private *scheduler = bthread_get_scheduler();
    volatile __bthread_private *bthread_private = (__bthread_private *) malloc(sizeof(__bthread_private));
    bthread_private->state = __BTHREAD_READY;
    bthread_private->attr = *attr;
    bthread_private->body = start_routine;
    bthread_private->arg = arg;
    bthread_private->stack = NULL;
    bthread_private->wake_up_time = 0;
    bthread_private->cancel_req = 0;
    bthread_private->quantumCounter = 0;
    bthread_private->priority = 1;
    tqueue_enqueue(&scheduler->queue, bthread_private);
    *bthread = tidCounter;
    bthread_private->tid = tidCounter;
    tidCounter++;
    printf("There are %lu threads \n", tqueue_size(scheduler->queue));
    return 0;
}

//Waits for the thread specified by bthread to terminate (i.e. __BTHREAD_ZOMBIE state), by
//scheduling all the threads. In the following we will discuss some details about this procedure.
int bthread_join(bthread_t bthread, void **retval) {
    printf("********************* Join del thread %lu\n", bthread);
    bthread_block_timer_signal();
    bthread_setup_timer();
    volatile __bthread_scheduler_private *scheduler = bthread_get_scheduler();
    scheduler->current_item = scheduler->queue;
    //printf("There are %lu threads \n", tqueue_size(scheduler->queue));
    save_context(scheduler->context);
    if (bthread_check_if_zombie(bthread, retval)) return 0;

    // Ricerco il primo thread con lo stato ready usando la politica di scheduling richiesta dall'utente
    scheduler->scheduling_routine();
    __bthread_private *tp = (__bthread_private *) tqueue_get_data(scheduler->current_item);
    //printf("There are %lu threads \n", tqueue_size(scheduler->queue));
    //printf("SCHEDULING Tp = %lu Stato %lu\n", tp->tid, tp->state);
    // Restore context or setup stack and perform first call
    if (tp->stack) {
        restore_context(tp->context);
    } else {
        tp->stack = (char *) malloc(sizeof(char) * STACK_SIZE);
#if __x86_64__
        asm __volatile__("movq %0, %%rsp"::
        "r"((intptr_t) (tp->stack + STACK_SIZE - 1)));
#else
        asm __volatile__("movl %0, %%esp" ::
        "r"((intptr_t) (tp->stack + STACK_SIZE - 1)));
#endif
        bthread_unblock_timer_signal();
        bthread_exit(tp->body(tp->arg));
    }
}

//Saves the thread context and restores (long-jumps to) the scheduler context. Saving the thread
//context is achieved using sigsetjmp, which is similar to setjmp but can also save the signal
//mask if the provided additional parameter is not zero (to restore both the context and the signal
//mask the corresponding call is siglongjmp). Saving and restoring the signal mask is required
//for implementing preemption.
void bthread_yield() {
    bthread_block_timer_signal();
    volatile __bthread_scheduler_private *scheduler = bthread_get_scheduler();
    // Ipotizzando di essere il thread corrente, dopo aver eseguito, non essendoci preemption, faccio yesld(). Se mi viene ritornato 0 allora "passo la palla"
    // allo scheduler. Se non mi viene ritornato 0 vuol dire che ho "dormito" fin'ora e che sono il thread che è stato scelto da eseguire quindi non ripasso
    // la palla allo scheduler ma eseguo
    volatile __bthread_private *thread = (__bthread_private *) tqueue_get_data(scheduler->current_item);
    if (save_context(thread->context) == 0)
        restore_context(scheduler->context);
    bthread_unblock_timer_signal();
}

//Terminates the calling thread and returns a value via retval that will be available to another
//thread in the same process that calls bthread_join, then yields to the scheduler. Between
//bthread_exit and the corresponding bthread_join the thread stays in the __BTHREAD_ZOMBIE state.
void bthread_exit(void *retval) {
    volatile __bthread_scheduler_private *scheduler = bthread_get_scheduler();
    volatile __bthread_private *t = tqueue_get_data(scheduler->current_item);
    t->state = __BTHREAD_ZOMBIE;
    t->retval = retval;
    bthread_yield();
}

void bthread_cleanup() {

}

double get_current_time_millis() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
}

int bthread_cancel(bthread_t bthread) {
    // cancella il thread settando il flag del thread passato a 1
    volatile __bthread_private *thread = (__bthread_private *) bthread_get_queue_at(bthread)->data;
    thread->cancel_req = 1;
}

void bthread_testcancel(void) {
    // controlla se deve suicidarsi
    volatile __bthread_scheduler_private *scheduler = bthread_get_scheduler();
    volatile __bthread_private *thread = (__bthread_private *) (scheduler->current_item->data);
    if (thread->cancel_req) {
        bthread_exit(-1);
    }
}


void bthread_sleep(double ms) {
    volatile __bthread_scheduler_private *scheduler = bthread_get_scheduler();
    volatile __bthread_private *thread = (__bthread_private *) (scheduler->current_item->data);
    thread->state = __BTHREAD_SLEEPING;
    thread->wake_up_time = get_current_time_millis() + ms;
    bthread_yield();
}

void bthread_block_timer_signal() {
    // creo una maschera e ci metto il segnale SIGVTALRM, la setto con
    if (mask == NULL) {
        mask = (sigset_t *) malloc(sizeof(sigset_t));
        sigemptyset(mask);
        sigaddset(mask, SIGVTALRM);
    }
    sigprocmask(SIG_BLOCK, mask, NULL);

}

void bthread_unblock_timer_signal() {
    // imposto la maschera di default
    sigprocmask(SIG_UNBLOCK, mask, NULL);
}

void bthread_printf(const char *format, ...) {
    bthread_block_timer_signal();
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    bthread_unblock_timer_signal();
}

void bthread_set_priority(int p) {
    volatile __bthread_scheduler_private *scheduler = bthread_get_scheduler();
    volatile __bthread_private *current = (__bthread_private *) tqueue_get_data(scheduler->current_item);
    if (p > 0) {
        current->priority = p;
    }
}

void bthread_set_schedulong_routine(void *(*bthread_scheduling_routine)(void *)) {
    volatile __bthread_scheduler_private *scheduler = bthread_get_scheduler();
    scheduler->scheduling_routine = bthread_scheduling_routine;
}
// Scheduling routines

void *bthread_round_robin_scheduling() {
    // vado a cercare il primo thread libero e lo assegno a current_item
    volatile __bthread_scheduler_private *scheduler = bthread_get_scheduler();
    volatile __bthread_private *tp;
    do {
        scheduler->current_item = tqueue_at_offset(scheduler->current_item, 1);
        tp = (__bthread_private *) tqueue_get_data(scheduler->current_item);
        // se dorme forse è il momento di svegliarsi
        if (tp->state == __BTHREAD_SLEEPING && tp->wake_up_time < get_current_time_millis()) {
            tp->wake_up_time = 0;
            tp->state = __BTHREAD_READY;
        }
    } while (tp->state != __BTHREAD_READY);
}

void *bthread_priority_scheduling() {
    volatile __bthread_scheduler_private *scheduler = bthread_get_scheduler();
    volatile __bthread_private *tp = (__bthread_private *) tqueue_get_data(scheduler->current_item);
    if (tp->state != __BTHREAD_READY || tp->quantumCounter == tp->priority) {
        if (tp->quantumCounter == tp->priority)
            tp->quantumCounter = 0;
        // se il thread corrente non è più ready oppure ha eseguito tutti i suoi quanti di tempo ne schedulo un altro con round robin
        bthread_round_robin_scheduling();
    } else {
        // altrimenti incremento il numero di quanti di tempo che ha eseguito finora.
        tp->quantumCounter++;
    }
}

void *bthread_random_scheduling() {


}

void *bthread_lottery_scheduling() {

}