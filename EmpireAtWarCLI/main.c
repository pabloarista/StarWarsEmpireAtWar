#include <stdio.h>
#include <stdlib.h>
#include "pmeg.h"
#include <conio.h>

#define TEST 0

#include <signal.h>
#include <errno.h>
#include <string.h>
#include <windows.h>
void print_err(char* signal, int code, char* desc) {
    printf("%s (%d) - %s\n", signal, code, desc);
}
void signal_handler(int code) {
    int errnum = errno;
        if(errnum) {
        fprintf(stderr, "Error #%d:\t", errnum);
        perror(0);
    }//if
    switch(code) {
        case SIGINT: print_err("SIGINT", code, "Interactive attention"); break;
        case SIGILL: print_err("SIGILL", code, "Illegal instruction"); break;
        case SIGFPE: print_err("SIGFPE", code, "Floating point error"); break;
        case SIGSEGV: print_err("SIGSEGV", code, "Segmentation violation"); break;
        case SIGTERM: print_err("SIGTERM", code, "Termination request"); break;
        case SIGBREAK: print_err("SIGBREAK", code, "Control-break"); break;
        case SIGABRT: print_err("SIGABRT", code, "Abnormal termination (abort)"); break;
        default: print_err("Unknown signal", code, "?"); break;
    }//switch
#if TEST
    printf("press any key to quit");
    getch();
#endif // TEST
    exit(-1);
}
int main(int argc, char** argv) {
    signal(SIGINT, signal_handler);
    signal(SIGILL, signal_handler);
    signal(SIGFPE, signal_handler);
    signal(SIGSEGV, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGBREAK, signal_handler);
    signal(SIGABRT, signal_handler);
    char* test = (char*)malloc(50);
    if(test) {
        strcpy(test, "hello world!");
        puts(test);
        free(test);
    }//if
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
#if TEST
    printf("Press any key to exit");
    getch();
#endif
    return result;
}
