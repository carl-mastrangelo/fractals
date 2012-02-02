
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>


#include "heap.h"


void heap_init(Heap * hp, heap_compare compar )
{
    pthread_mutex_init(&hp->lock, NULL);
    pthread_cond_init(&hp->cond, NULL);
    hp->size = 1;
    hp->capacity = 1;
    hp->data = NULL;
    
    hp->compar = compar;
}

void heap_push(Heap * hp, void * elem)
{
    unsigned int i;
    pthread_mutex_lock(&hp->lock);
    
    if( hp->size >= hp->capacity )
    {
        void ** temp;
        hp->capacity *= 2;
        
        if( !(temp = realloc(hp->data, sizeof(void *) * hp->capacity) ) )
        {
            fprintf(stderr, "Memory error: heap_push\n");
            exit(EXIT_FAILURE);
        }
        else
        {
            hp->data = temp;
            hp->data[0] = NULL;
        }
    }
    
    assert(hp->size < hp->capacity);
    
    hp->data[hp->size] = elem;
    
    i = hp->size++;
    while(i > 1)
    {
        if( hp->compar(hp->data[i], hp->data[i/2]) < 0)
        {
            void * temp;
            temp = hp->data[i/2];
            hp->data[i/2] = hp->data[i];
            hp->data[i] = temp;
            i /= 2;
        }
        else
        {
            break;
        }
    }

    pthread_cond_broadcast(&hp->cond);
    pthread_mutex_unlock(&hp->lock);
}

void * heap_peek(Heap * hp)
{
    void * elem;
    pthread_mutex_lock(&hp->lock);
    
    while(hp->size <= 1)
    {
        pthread_cond_wait(&hp->cond, &hp->lock);
    }
    
    elem = hp->data[1];
    
    pthread_mutex_unlock(&hp->lock);
    
    return elem;
}

void * heap_pop(Heap * hp)
{
    void * elem;
    int i, child, rchild;
    pthread_mutex_lock(&hp->lock);

    while(hp->size <= 1)
    {
        pthread_cond_wait(&hp->cond, &hp->lock);
    }
    
    elem = hp->data[1];
    
    
    
    
    
    hp->data[1] = hp->data[--hp->size];

    i = 1; child = i * 2;
    
    
    while( child < hp->size )
    {
        rchild = child + 1;
        if( rchild < hp->size && hp->compar(hp->data[rchild], hp->data[child]) < 0)
        {
            child = rchild;
        }
        if( hp->compar(hp->data[i], hp->data[child] ) > 0 )
        {
            void * temp = hp->data[child];
            hp->data[child] = hp->data[i];
            hp->data[i] = temp;
            i = child;
            child = i * 2;
        }
        else
        {
            break;
        }
    }
    
    if( hp->size < hp->capacity / 2)
    {
        void ** temp;
        hp->capacity /= 2;
        
        if( !(temp = realloc(hp->data, sizeof(void *) * hp->capacity) ) )
        {
            fprintf(stderr, "Memory error: heap_pop\n");
            exit(EXIT_FAILURE);
        }
        else
        {
            hp->data = temp;
        }
    }


    pthread_mutex_unlock(&hp->lock);
    return elem;
}

unsigned int heap_size(Heap * hp)
{
    unsigned int size;
    pthread_mutex_lock(&hp->lock);
    size = hp->size - 1 ;
    pthread_mutex_unlock(&hp->lock);
    return size;
}

void heap_wait(Heap * hp, void * needle)
{
    pthread_mutex_lock(&hp->lock);
    while(hp->size <= 1 || hp->compar(needle, hp->data[1]) != 0 )
    {
        pthread_cond_wait(&hp->cond, &hp->lock);
    }
    
    pthread_mutex_unlock(&hp->lock);
}

#ifdef HEAP_MAIN

int intcompare(const void *a, const void * b)
{
    const int *aa, *bb;
    
    aa  = a; bb = b;

    return *aa - *bb;
}


int main(int argc, char** argv)
{

    int * data, *out, i;
    
    Heap hp;
    heap_init(&hp, intcompare);

    assert(data = malloc(sizeof(int) * 100000) );
    assert(out = malloc(sizeof(int) * 100000) );
    for(i = 0; i < 100000; i++)
    {
        data[i] = rand();
        heap_push(&hp, data + i);
    }
    i = 0;
    
    while( heap_size(&hp) )
    {
        int * elem;
        elem = heap_pop(&hp);
        out[i++] = *elem;
    }
    
    for( i = 1; i < 100000; i++)
    {
        if(out[i-1] > out[i])
        {
            printf("Error! %d is bigger than %d\n", out[i-1], out[i]);
        }
    }
    
    printf("Size %d cap %d\n", hp.size, hp.capacity);

    
    
    return 0;
}

#endif

