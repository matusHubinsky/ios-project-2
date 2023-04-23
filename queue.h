#ifndef QUEUE_H
#define QUEUE_H

#define QUEUE_SIZE 64;

#include <semaphore.h>

struct node_t {
    sem_t* data;
    struct node_t* next;
};

struct queue_t {
    struct node_t* front;
    struct node_t* back;
};

bool is_empty(struct queue_t* queue);
struct queue_t* queue_init();
void queue_push(struct queue_t* queue, sem_t* data);
sem_t* queue_pop(struct queue_t* queue);
void queue_destroy(struct queue_t* queue);

#endif