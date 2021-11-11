//
// Created by daniele on 11/11/21.
//

#include <connectionQueue.h>

ConnectionQueue* initQueue(){

    ConnectionQueue *q = malloc(sizeof(ConnectionQueue)); //Alloco spazio per cosa

    if(!q) return NULL;

    q->head = malloc(sizeof(Node));
    q->head->sfd = -1;
    q->head->next = NULL;

    q->tail = q->head;
    q->len = 0;

    return q;
}

int push(ConnectionQueue *q, int sfd){

    if( (q == NULL) || (sfd == -1))
        return -1;

    Node *n = malloc(sizeof(Node));
    if(!n) return -1;
    n->sfd = sfd;
    n->next = NULL;

    /* Qui inizia la sezione critica */
    pthread_mutex_lock(&q->qlock);
    q->tail->next = n;
    q->tail = n;
    q->len++;
    pthread_mutex_unlock(&q->qlock);
    /* Qui finisce la sezione critica */

    return(q->len);

}

int pop(ConnectionQueue *q){

    int data;

    if(q == NULL) return -1;

    //Inizio sezione critica
    pthread_mutex_lock(&q->qlock);

    //Attendo fino a che non ci sono dei nuovi elementi nella coda
    while(q->head == q->tail)
        pthread_cond_wait(&q->qcond, &q->qlock);

    Node *n = q->head;
    data = n->sfd;
    q->head = n->next;
    q->len--;

    pthread_mutex_unlock(&q->qlock);
    //Fine sezione critica

    //Libero la memoria del nodo rimosso
    free(n);

    return data;
}