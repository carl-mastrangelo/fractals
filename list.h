#ifndef LIST_H
#define LIST_H

struct Node
{
    struct Node * next;
    void * data;
};

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t cond;
    struct Node * head, *tail;
    int size;
} List;

void list_init(List * ls);
void list_add(List * ls, void * data);
void * list_getmin(List * ls, int (*compar)(const void *a, const void*b), void * match );
void list_remove(List * ls, void * elem);
int list_size(List * ls);


#endif
