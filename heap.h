#ifndef HEAP_H
#define HEAP_H

typedef int (*heap_compare)(const void * a, const void * b);

typedef struct Heap 
{
    pthread_mutex_t lock;
    pthread_cond_t cond;
    unsigned int size;
    unsigned int capacity;
    void ** data;
    heap_compare compar;
} Heap;

void heap_init(Heap * hp, heap_compare compar );

void heap_push(Heap * hp, void * elem);
void * heap_peek(Heap * hp);
void * heap_pop(Heap * hp);
unsigned int heap_size(Heap * hp);

void heap_wait(Heap * hp, void * needle);

#endif

