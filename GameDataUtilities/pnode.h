#ifndef PNODE_H_INCLUDED
#define PNODE_H_INCLUDED

struct pnode {
    void* val;
    struct pnode* next;
};

struct pnode* pnode_create(void* val);
void pnode_destroy(struct pnode** node, void(*destroy_val_fn)(void** val));

#endif // PNODE_H_INCLUDED
