#ifndef PLINKED_LIST_H_INCLUDED
#define PLINKED_LIST_H_INCLUDED

#include "pnode.h"
#include <stdbool.h>

struct plinked_list {
    struct pnode* head;
    struct pnode* tail;
    void(*destroy_val_fn)(void** val);
    int count;
};

struct plinked_list* plinked_list_create(struct pnode* head, void(*destroy_val_fn)(void** val));
void plinked_list_destroy(struct plinked_list** ll);
bool plinked_list_did_insert_node(struct plinked_list* ll, struct pnode* n);
bool plinked_list_did_add_node(struct plinked_list* ll, struct pnode* n);
bool plinked_list_did_remove_node(struct plinked_list* ll, struct pnode** n);
bool plinked_list_did_remove_head(struct plinked_list* ll);
bool plinked_list_did_remove_tail(struct plinked_list* ll);
bool plinked_list_is_empty(struct plinked_list* ll);


#endif // PLINKED_LIST_H_INCLUDED
