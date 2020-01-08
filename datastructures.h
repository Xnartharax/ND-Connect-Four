#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H
#include <stdlib.h>
typedef struct node{
    struct node * next;
    void * data;
} node_t;
typedef struct linked_list{
    node_t * head;
    node_t * tail;
    int length;
} linked_list_t;
typedef struct hashlist {
    linked_list_t * chain_array;
    int length;
    int entries;
    unsigned int (*hash)(void *);
    int (*compare)(void *, void *);
} hashlist_t;
void append_to_list(linked_list_t * linked_list, void * toAppend){
    node_t * new_node = (node_t *) malloc(sizeof(node_t));
    new_node->data = toAppend;
    if(!linked_list->tail){
        linked_list->tail = new_node;
        linked_list->head = new_node;
    }
    else{
        linked_list->tail->next = new_node;
        linked_list->tail = new_node;
    }
    linked_list->length++;
}
void append_to_hashlist(hashlist_t * hashlist, void * toAppend){
    unsigned int index = hashlist->hash(toAppend) % hashlist->length;
    append_to_list(&hashlist->chain_array[index], toAppend);
    hashlist->entries++;
    if (hashlist->entries > hashlist->length*3){
        int new_length = hashlist->length+5;
        linked_list_t * new_chain_array = (linked_list_t *) malloc(new_length*sizeof(linked_list_t)); 
        for (int i = 0; i < new_length; i++)
        {
            new_chain_array[i].head = NULL;
            new_chain_array[i].tail = NULL;
            new_chain_array[i].length = 0;
        }
        for (int i = 0; i < hashlist->length; i++)
        {
            node_t * current_node = hashlist->chain_array[i].head;
            while(current_node){
                unsigned int index = hashlist->hash(current_node->data) % new_length;
                append_to_list(&new_chain_array[index], current_node->data);
                node_t * next_node = current_node->next;
                free(current_node->data);
                free(current_node);
                current_node = next_node;
            }
        }
        free(hashlist->chain_array);
        hashlist->chain_array = new_chain_array;
        
    }
}
int lookup_hashlist(hashlist_t * hashlist, int coords[]){
    unsigned int index = hashlist->hash((void *) coords) % hashlist->length;
    node_t * current_node  = hashlist->chain_array[index].head;
    while(current_node){
        if(hashlist->compare(current_node->data, (void *) coords)==1){
            return 1;
        }
        current_node = current_node->next;
    }
    return 0;
}
#endif