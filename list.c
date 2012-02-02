#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#include "list.h"

void list_init(List * ls)
{
    pthread_mutex_init(&ls->lock, NULL);
    pthread_cond_init(&ls->cond, NULL);
    ls->size = 0;
    ls->head = ls->tail = NULL;
}

void list_add(List * ls, void * data)
{
    struct Node * temp;
    if( !(temp = malloc(sizeof(struct Node) ) ) )
    {
        fprintf(stderr, "Out of memory in list_add\n");
        exit(1);
    }
    
    temp->data = data;
    temp->next = NULL;
    
    
    pthread_mutex_lock(&ls->lock);
    while( ls->size >= 4000)
    {
        pthread_cond_wait(&ls->cond, &ls->lock);
    }
    {
        if(ls->head == NULL)
        {
            ls->head = temp;
        }
        if(ls->tail)
        {
            ls->tail->next = temp;
        }

        ls->tail = temp;
        ls->size += 1;
    }
    pthread_cond_broadcast(&ls->cond);
    pthread_mutex_unlock(&ls->lock);
}

void * list_getmin(List * ls, int (*compar)(const void *a, const void*b), void * match )
{
    struct Node * min = NULL;
    
    pthread_mutex_lock(&ls->lock);
    while(ls->size == 0)
    {
        pthread_cond_wait(&ls->cond, &ls->lock);
    }
    {
        struct Node * temp;
        if(ls->size == 0)/* this should not occur */
        {
            pthread_mutex_unlock(&ls->lock);
            fprintf(stderr, "NO!\n");
            return min;
        }
        
        min = ls->head;
        
        if(ls->size == 1)
        {
            
            pthread_mutex_unlock(&ls->lock);
            return min->data;
        }
        
        temp = ls->head->next;
        
        while(temp)
        {
            if( match && (compar(temp->data, match) == 0) )
            {
                pthread_mutex_unlock(&ls->lock);

                return temp->data;
            }
        
            if( compar(min->data, temp->data) > 0)
            {
                min = temp;
            }
            temp = temp->next;
        }

    }
    pthread_mutex_unlock(&ls->lock);

    return min->data;
}


void list_remove(List * ls, void * elem)
{
    struct Node * cur, *prev;

    
    pthread_mutex_lock(&ls->lock);
    {
        cur = ls->head;
        prev = NULL;

        while(cur)
        {
            if(cur->data == elem)
            {
                if( cur == ls->head )
                {
                    ls->head = ls->head->next;
                }
                if( cur == ls->tail )
                {
                    ls->tail = prev;
                }
                if( prev )
                {
                    prev->next = cur->next;
                }
                ls->size -= 1;
                free(cur);
                
                break;
            }
            
            prev = cur;
            cur = cur->next;
        }
    }
    pthread_cond_broadcast(&ls->cond);
    pthread_mutex_unlock(&ls->lock);
    
}

int list_size(List * ls)
{
    return ls->size;
}




