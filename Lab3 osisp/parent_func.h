#pragma once
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#define P_OPTION_DELAY_TIME 5

int kill(pid_t pid, int sig);

typedef enum{
    CREATE_OPTION,
    DELETE_OPTION,
    L_OPTION,
    K_OPTION,
    S_OPTION,
    G_OPTION,
    S_NUM_OPTION,
    G_NUM_OPTION,
    P_NUM_OPTION,
    Q_OPTION
}OPTIONS;

typedef struct {
    int is_stoped;      //статус процесса
    pid_t pid;          //ID процесса
    char name[10];      //название
}child_struct;

OPTIONS get_option(char*,int);
void create_child();
void delete_last_child();
void print_all(char*);
void delete_all();
void ban_all_stat();
void allow_all_stat();
int read_number_from_str(char* start, char* end) ;
void ban_stat(int);
void allow_stat(int);
void request_stat(int num);