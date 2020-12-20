#ifndef PFILE_H_INCLUDED
#define PFILE_H_INCLUDED

#include <stdio.h>
#include <stdbool.h>

struct pfile {
    char* file_name;
    char* root_path;
    char** file_names;
    int file_names_len;
    FILE* fp;
};
//bool pfile_valid(struct pfile* pf);
struct pfile* pfile_create(char* file_name);
void pfile_destroy(struct pfile** pf);
bool pfile_create_file_names(struct pfile* pf, int n);


#endif // PFILE_H_INCLUDED
