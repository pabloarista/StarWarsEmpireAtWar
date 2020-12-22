#include "pfile.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

static char* pfile_get_root_path(char* file_name);

char* pfile_get_root_path(char* file_name) {
    char* tmp = file_name;
    char* ptr = 0;
    if(file_name) {
        while((tmp = strchr(tmp, '\\'))) {
            ptr = tmp = tmp + 1;
        }//while
    }//if
    char* root_path;
    if (ptr) {
        size_t n = ptr - file_name;
        assert(n > 0);
        root_path = (char*)malloc(sizeof *root_path * (n + 1));
        if(root_path) {
            strncpy(root_path, file_name, n);
            printf("full path:\t%s\nslice:\t%s\n", file_name, root_path);
            *(root_path + n) = 0;
        } else perror("out of memory!\n");
    } else {
        perror("error getting root path\n");
        root_path = 0;
    }//else

    return root_path;
}

struct pfile* pfile_create(char* file_name) {
    char* file_name_cp = strdup(file_name);
    char* root_path;
    if(file_name_cp) root_path = pfile_get_root_path(file_name);
    else root_path = 0;
    struct pfile* p;
    if(root_path) p = (struct pfile*)malloc(sizeof *p);
    else p = 0;
    if(p) {
        (*p).file_name = file_name_cp;
        (*p).root_path = root_path;
        (*p).file_names = 0;
        (*p).file_names_len = 0;
        (*p).fp = 0;
    } else {
        perror("out of memory!\n");
        if(file_name_cp) free(file_name_cp);
        if(root_path) free(root_path);
    }//else

    return p;
}
void pfile_destroy(struct pfile** pf) {
    struct pfile* current = *pf;
    if(current) {
        if((*current).file_name) free((*current).file_name);
        (*current).file_name = 0;
        if((*current).root_path) free((*current).root_path);
        (*current).root_path = 0;
        int i = 0;
        char** file_names = (*current).file_names;
        int n;
        if(file_names) n = (*current).file_names_len;
        else n = 0;
        for(; i < n; ++i) {
            char* file_name = *(file_names + i);
            if(file_name) free(file_name);
            *(file_names + i) = 0;
            file_name = 0;
        }//for
        if(file_names) free(file_names);
        file_names = 0;
        (*current).file_names = 0;
        (*current).file_names_len = 0;
        if((*current).fp) fclose((*current).fp);
        (*current).fp = 0;
    }//if
}
bool pfile_create_file_names(struct pfile* pf, int n) {
    bool did_create;
    assert(n > 0);
    if(n > 0) {
        char** file_names = (char**)malloc(sizeof *file_names * n);
        if(!file_names) did_create = false;
        else {
            memset(file_names, 0, sizeof *file_names * n);
            did_create = true;
            (*pf).file_names = file_names;
            (*pf).file_names_len = n;
        }//else
    } else did_create = false;

    return did_create;
}
