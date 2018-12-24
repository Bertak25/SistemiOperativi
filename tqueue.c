//
// Created by edoardo on 18/10/18.
//
#include "tqueue.h"
#include <stdlib.h>
#include <stdio.h>

/* Adds a new element at the end of the list, returns its position */
// q punta alla testa della lista quindi se vogliamo operare sulla testa della lista dobbiamo deferenziare q. (*q) equivale alla testa!
unsigned long int tqueue_enqueue(TQueue *q, void *data) {
    if ((*q) == NULL) {
        // La coda e' vuota, la instanzio e aggiungo l'elemento
        (*q) = (TQueue) malloc(sizeof(TQueue));
        (*q)->data = data;
        (*q)->next = (*q);
        return 0;
    }
    //La coda non e' vuota, trovo la fine
    TQueueNode *head = (*q);
    unsigned long int position = tqueue_size((*q)) - 1;
    TQueueNode *tail = tqueue_at_offset((*q), position);
    // aggiungo l'elemento in coda e faccio puntare la testa dal nuovo elemento aggiunto
    TQueueNode *new = malloc(sizeof(TQueueNode));
    tail->next = new;
    new->data = data;
    new->next = head;
    return ++position;
}

/* Removes and returns the element at the beginning of the list, NULL if the queue is empty */
void *tqueue_pop(TQueue *q) {
    if ((*q) == NULL)
        return NULL;
    // Mi salvo i dati da ritornare
    void *data = (*q)->data;
    // La vecchia testa è l'elemento puntato da (*q)
    TQueueNode *oldHead = (*q);
    // La nuova testa sarà l'elemento successivo di quella vecchia
    TQueueNode *newHead = oldHead->next;
    // Controllo che la coda non si sia svuotata (è il caso in cui faccio la pop dell'ultimo valore)
    if (oldHead == newHead) {
        free(oldHead);
        (*q) = NULL;
    } else {
        // Se la coda ha più di un elemento vado a recuperare l'ultimo elemento
        TQueueNode *tail = tqueue_at_offset(oldHead, (tqueue_size(oldHead) - 1));
        // faccio puntare l'ultimo elemento alla nuova testa
        tail->next = newHead;
        // Aggiorno il puntatore alla coda dopo aver effettuato tutte le modifiche
        (*q) = newHead;
        // cancello la vecchia testa
        free((oldHead));
    }
    // ritorno i dati che erano nella vecchia testa
    return data;
}

/* Returns the number of elements in the list */
unsigned long int tqueue_size(TQueue q) {
    // Se la coda è vuota ritorno 0
    if (q == NULL)
        return 0;
    TQueueNode *head = q;
    TQueueNode *current = q;
    unsigned long int size = 1;
    // Altrimenti itero finche non ritrovo la testa
    while (current->next != head) {
        current = current->next;
        size++;
    }
    return size;
}

/* Returns a 'view' on the list starting at (a positive) offset distance, NULL if the queue is empty */
TQueue tqueue_at_offset(TQueue q, unsigned long int offset) {
    if (q == NULL)
        return NULL;
    unsigned long int position = 0;
    TQueueNode *current = q;
    // Itero fino alla posizione specificata poi ritorno l'elemento corrispondente
    while (position != offset) {
        current = current->next;
        ++position;
    }
    return current;
}

/* Returns the data on the first node of the given list */
void *tqueue_get_data(TQueue q) {
    // Ritorno i dati contenuti nella testa, se vuota NULL
    if (q == NULL)
        return NULL;
    return q->data;
}

void tqueue_printInteger(TQueue q) {
    if (q == NULL) {
        printf("[ ]");
    } else {
        TQueueNode *head = q;
        TQueueNode *current = q;
        printf("[");
        // Altrimenti itero finche non ritrovo la testa
        do {
            printf(" %d, ", *((int *) (current->data)));
            current = current->next;
        } while (current != head);
        printf("]\n");
    }
}