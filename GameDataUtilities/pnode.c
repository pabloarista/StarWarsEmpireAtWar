#include "pnode.h"
#include <stdlib.h>

struct pnode* pnode_create(void* val) {
    struct pnode* node = (struct pnode*)malloc(sizeof *node);
    if(node) {
        (*node).val = val;
        (*node).next = 0;
    }//if

    return node;
}
void pnode_destroy(struct pnode** node, void(*destroy_val_fn)(void** val)) {
    if(*node) {
        (**node).next = 0;
        if((**node).val) destroy_val_fn(&((**node).val));
        (**node).val = 0;
        free(*node);
        *node = 0;
    }//if
}
