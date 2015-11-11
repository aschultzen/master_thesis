#include "linked_list.h"

node_t* create_node()
{
    node_t * new_node = malloc(sizeof(node_t));
    if(new_node == NULL) {
        die(10, "Malloc failed!\n");
    }
    return new_node;
}

void push(node_t * head, void * pointer) {
    node_t * current = head;
    while (current->next != NULL) {
        current = current->next;
    }

    /* now we can add a new variable */
    current->next = malloc(sizeof(node_t));
    current->next->pointer = pointer;
    current->next->next = NULL;
}

int pop(node_t ** head) {
    node_t * next_node = NULL;

    if (*head == NULL) {
        return -1;
    }

    next_node = (*head)->next;
    free(*head);
    *head = next_node;

    return 1;
}

int remove_node(node_t ** head, node_t ** remove_me) {
    node_t * current = *head;
    node_t * temp_node = NULL;

    if (head == remove_me) {
        return pop(head);
    }

    while(current != NULL){
        if(current->next == *remove_me){
            temp_node = current->next;
            current->next = temp_node->next;
            free(temp_node);
            return 1;
        }
    }
    return -1;
}

void print_list(node_t * head) {
    node_t * current = head;

    while (current != NULL) {
        #pragma GCC diagnostic ignored "-Wformat="
        printf("%d\n", current);
        current = current->next;
    }
}

node_t* create_list()
{
    node_t *head = create_node();
    return head;
}


