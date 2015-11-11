#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdio.h>
#include <stdlib.h>

// Mine
#include "utils.h"

struct node{
    void *pointer;
    struct node * next;
} __attribute__ ((packed));

typedef struct node node_t;

#endif /* !LINKED_LIST_H */