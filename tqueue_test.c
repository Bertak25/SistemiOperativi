//#include <stdio.h>
//#include <stdlib.h>
//#include "tqueue.h"
//
//int main() {
//    TQueue queue = NULL;
//    int *data10 = malloc(sizeof(int));
//    *data10 = 10;
//    int *data20 = malloc(sizeof(int));
//    *data20 = 20;
//    int *data30 = malloc(sizeof(int));
//    *data30 = 30;
//
//    tqueue_enqueue(&queue, data10);
//    tqueue_enqueue(&queue, data20);
//    tqueue_enqueue(&queue, data30);
//    tqueue_printInteger(queue);
//    printf("Dimensione di TQueue: %d\n", tqueue_size(queue));
//
//    printf("\nFaccio una pop, valore: %d\n", *((int *) tqueue_pop(&queue)));
//    printf("Dimensione di TQueue dopo la pop: %d\n", tqueue_size(queue));
//    tqueue_printInteger(queue);
//
//    printf("\nFaccio una seconda pop, valore: %d\n", *((int *) tqueue_pop(&queue)));
//    printf("Dimensione di TQueue dopo la seconda pop: %d\n", tqueue_size(queue));
//    tqueue_printInteger(queue);
//
//    printf("\nFaccio una terza pop, valore: %d\n", *((int *) tqueue_pop(&queue)));
//    printf("Dimensione di TQueue dopo la terza pop: %d\n", tqueue_size(queue));
//    tqueue_printInteger(queue);
//
//    printf("\n\nRiaggiungo tutti i valori...\n");
//    tqueue_enqueue(&queue, data10);
//    tqueue_enqueue(&queue, data20);
//    tqueue_enqueue(&queue, data30);
//    tqueue_printInteger(queue);
//    printf("Dimensione di TQueue: %d\n", tqueue_size(queue));
//
//    return 0;
//}