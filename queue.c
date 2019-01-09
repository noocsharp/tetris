#include "queue.h"
#include <SDL2/SDL.h>
#include <string.h>
#include <stddef.h>

int initQueue(struct Queue* queue, struct Piece* array, int capacity) {
    queue->front = 0;
    queue->back = 0;
    queue->size = 0;
    queue->capacity = capacity;
    queue->array = array;
}

int destroyQueue(struct Queue* queue) {
    memset(queue->array, 0, queue->capacity);
}

int enqueue(struct Queue* queue, struct Piece piece) {
    if (queue->size+1 > queue->capacity)
        return -1;

    memcpy(&queue->array[queue->back], &piece, sizeof(struct Piece));
    queue->back = (queue->back+1)%queue->capacity;
    queue->size = queue->size + 1;
    return 0;
}

struct Piece* dequeue(struct Queue* queue) {
    if (queue->size <= 0)
        return NULL;

    struct Piece* p = &(queue->array[queue->front]);
    queue->front = (queue->front+1)%queue->capacity;
    queue->size = queue->size - 1;
    return p;
}
