//
// Created by daniele on 11/11/21.
//

#include "queue.h"

Queue* initQueue(){

    Queue *q = malloc(sizeof(Queue)); //Alloco spazio per coda

    if(!q) return NULL;

    q->head = malloc(sizeof(Node));
    q->head->data = NULL;
    q->head->next = NULL;

    q->tail = q->head;
    q->len = 0;

    return q;
}

int push(Queue *q, void *data){

    if( (q == NULL) || (data == NULL))
        return -1;

    Node *n = malloc(sizeof(Node));
    if(!n) return -1;
    n->data = data;
    n->next = NULL;

    /* Qui inizia la sezione critica */
    pthread_mutex_lock(&q->qlock);
    q->tail->next = n;
    q->tail = n;
    q->len++;
    //Segnalo l inserimento di un nuovo elemento nella coda agli altri thread
    pthread_cond_signal(&q->qcond);
    pthread_mutex_unlock(&q->qlock);
    /* Qui finisce la sezione critica */

    return(q->len);

}

void *pop(Queue *q){

    void *data;

    if(q == NULL){
        printf("[POP] queue pointer is null!\n");
        return NULL;
    }

    //Inizio sezione critica
    pthread_mutex_lock(&q->qlock);

    //Attendo fino a che non ci sono dei nuovi elementi nella coda
    while(q->head == q->tail)
        pthread_cond_wait(&q->qcond, &q->qlock);

    Node *n = q->head;
    data = q->head->next->data;
    q->head = n->next;
    q->len--;

    pthread_mutex_unlock(&q->qlock);
    //Fine sezione critica

    //Libero la memoria del nodo rimosso
    free(n);

    return data;
}