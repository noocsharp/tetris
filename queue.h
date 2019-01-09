#include "piece.h"

struct Queue {
    int front, back, size;
    unsigned capacity;
    struct Piece* array;
};

int initQueue(struct Queue *queue, struct Piece* array, int capacity);
int destroyQueue(struct Queue *queue);
int enqueue(struct Queue* queue, struct Piece piece);
struct Piece* dequeue(struct Queue* queue);
