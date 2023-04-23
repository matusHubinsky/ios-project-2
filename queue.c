#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#include "queue.h"


bool is_empty(struct queue_t* queue) {
    return (queue->front == NULL) ? true : false;
}


static struct node_t* create_node(sem_t* data) {
    struct node_t* new_node = malloc(sizeof(struct node_t));
    if (new_node == NULL) {
        fprintf(stderr, "Error allocating memory for node");
        exit(1);
    }
    new_node->data = data;
    new_node->next = NULL;
    return new_node;
}


struct queue_t* queue_init() {
    struct queue_t* new_queue = malloc(sizeof(struct queue_t));
    if (new_queue == NULL) {
        fprintf(stderr, "Error allocating memory for queue");
        exit(1);
    }

    new_queue->front = NULL;
    new_queue->back = NULL;
    return new_queue;
}


void queue_push(struct queue_t* queue, sem_t* data) {
    struct node_t* new_node = NULL; 
    new_node = create_node(data);
    if (queue->back == NULL) {
        queue->front = new_node;
        queue->back = new_node;
        return;
    }
    queue->back->next = new_node;
    queue->back = new_node;
}


sem_t* queue_pop(struct queue_t* queue) {
    if (is_empty(queue)) {
        fprintf(stderr, "Error: queue is empty");
        exit(1);
    }
    
    struct node_t* tmp_node = queue->front;
    sem_t* data = tmp_node->data;
    queue->front = tmp_node->next;
    if (queue->front == NULL) {
        queue->back = NULL;
    }
    free(tmp_node);
    return data;
}


void queue_destroy(struct queue_t* queue) {
    while (!is_empty(queue)) {
        queue_pop(queue);
    }
    free(queue);
}

