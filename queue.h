#ifndef QUEUE_H
#define QUEUE_H

#define QUEUE_SIZE 64;

struct node_t {
    int data;
    struct node_t* next;
};

struct queue_t {
    struct node_t* front;
    struct node_t* back;
};

struct queue_t* queue_init();
void queue_push(struct queue_t* queue, int data);
int queue_pop(struct queue_t* queue);
void queue_destroy(struct queue_t* queue);
void queue_print(struct queue_t* queue);

#endif