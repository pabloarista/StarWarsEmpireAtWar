#include <stdio.h>
#include <stdlib.h>
#include "pmeg.h"
#include <conio.h>

#define TEST 0

#include <signal.h>
#include <errno.h>
#include <string.h>
#include <windows.h>

void abort_handler(int code) {
    //perror(0);
    int errnum = errno;
    fprintf(stderr, "Value of errno: %d\n", errno);
    perror("Error printed by perror");
    fprintf(stderr, "Error opening file: %s\n", strerror( errnum ));
    printf("signal %d was triggered\n", code);
#if TEST
    printf("press any key to quit");
    getch();
#endif
}
int main(int argc, char** argv) {
    signal(SIGABRT, abort_handler);
    signal(SIGTERM, abort_handler);
    signal(SIGSEGV, abort_handler);
#if !TEST
#warning TODO:we could other options: 1) add file to meg file 2) create meg file with a list of files or a directory to add
    if (argc < 2) {
        printf("no input file! unable to process\nusage:\t%s <full meg file path to extract>\nPress any key to exit", *argv);
        getch();
        return -1;
    }//if
#warning TODO:ensure that the path is a full path
    char* file_name = *(argv + 1);
#else
    const DWORD N = 1024;
    char file_name[N];
    file_name[0] = 0;
    GetEnvironmentVariable("USERPROFILE", file_name, N);
    puts(file_name);
    strcat(file_name, "\\Desktop\\tmp\\Config.meg");
    puts(file_name);
#endif
    int result = 0;
    pmeg_process(file_name, &result);
    printf("Press any key to exit");
    getch();

    return result;
}
