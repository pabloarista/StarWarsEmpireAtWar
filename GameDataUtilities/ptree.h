#ifndef P_TREE_H_INCLUDED
#define P_TREE_H_INCLUDED
#include <stdbool.h>

struct ptree {
    void* val;
    struct ptree* next;
    struct ptree* children;
};
struct ptree* ptree_create(void* val);
void ptree_destroy(struct ptree** branch, void(*destroy_val_fn)(void** val));
//if sibling IS NULL, then the reference will be the last child of branch OR inserting if there are NO children
bool ptree_append(struct ptree* branch, struct ptree* child, struct ptree* sibling);
//if sibling IS NULL, then the reference will be the first child of branch OR inserting if there are NO children
bool ptree_insert(struct ptree* branch, struct ptree* child, struct ptree* sibling);
bool ptree_remove(struct ptree* branch, struct ptree* child, struct ptree* sibling);
int ptree_count(struct ptree* branch);

#endif // P_TREE_H_INCLUDED
