#include "plinked_list.h"
#include <stdlib.h>

struct plinked_list* plinked_list_create(struct pnode* head, void(*destroy_val_fn)(void** val)) {
    struct plinked_list* ll = (struct plinked_list*)malloc(sizeof *ll);
    if(ll) {
        (*ll).head = (*ll).tail = head;
        (*ll).destroy_val_fn = destroy_val_fn;
    }//if

    return ll;
}
void plinked_list_destroy(struct plinked_list** ll) {
    if(*ll) {
        struct pnode* current = (**ll).head;
        while(current) {
            struct pnode* tmp = current;
            current = (*current).next;
            pnode_destroy(&tmp, (**ll).destroy_val_fn);
        }//while
        (**ll).head = 0;
        (**ll).tail = 0;
        free(*ll);
        *ll = 0;
    }//if
}
bool plinked_list_did_insert_node(struct plinked_list* ll, struct pnode* n) {
    bool did_insert;
    if(ll && (*ll).head && n) {
        (*n).next = (*ll).head;
        (*ll).head = n;
        did_insert = true;
    } else {
        did_insert = false;
    }//else

    return did_insert;
}
bool plinked_list_did_add_node(struct plinked_list* ll, struct pnode* n) {
    bool did_add;
    if(ll && (*ll).tail && n) {
        (*(*ll).tail).next = n;
        (*ll).tail = n;
        did_add = true;
    } else {
        did_add = false;
    }//else

    return did_add;
}
bool plinked_list_did_remove_node(struct plinked_list* ll, struct pnode** n) {
    bool did_remove = false;
    if(ll && *n) {
        if(*n == (*ll).head) {
            did_remove = plinked_list_did_remove_head(ll);
        } else if (*n == (*ll).tail) {
            did_remove = plinked_list_did_remove_tail(ll);
        } else {
            struct pnode* current = (*ll).head;
            struct pnode* prev = 0;
            while(current) {
                if(current == *n) {
                    if(prev) (*prev).next = (*current).next;
                    pnode_destroy(n, (*ll).destroy_val_fn);
                    did_remove = true;
                    break;
                }//if
                prev = current;
                current = (*current).next;
            }//while
        }//else
    }//if

    return did_remove;
}
bool plinked_list_did_remove_head(struct plinked_list* ll) {
    bool did_remove;
    if(ll && (*ll).head) {
        struct pnode* h = (*ll).head;
        (*ll).head = (*h).next;
        pnode_destroy(&h, (*ll).destroy_val_fn);
        did_remove = true;
    } else {
        did_remove = false;
    }//else

    return did_remove;
}
bool plinked_list_did_remove_tail(struct plinked_list* ll) {
    bool did_remove = false;
    if(ll && (*ll).tail) {
        struct pnode* t = (*ll).tail;
        struct pnode* current = (*ll).head;
        while(current) {
            if((*current).next == t) {
                (*ll).tail = current;
                (*current).next = 0;
                did_remove = true;
                break;
            }//if
        }//while
    }//if

    return did_remove;
}
bool plinked_list_is_empty(struct plinked_list* ll) {
    bool is_empty = !ll || !(*ll).head;

    return is_empty;
}
