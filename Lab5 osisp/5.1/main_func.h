#pragma once
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include "general_func.h"
#include <pthread.h>
#include <threads.h>
#include <semaphore.h>

typedef enum{
    CREATE_PRODUCER_OPTION,
    DELETE_PRODUCER_OPTION,
    CREATE_CONSUMER_OPTION,
    DELETE_CONSUMER_OPTION,
    QUEUE_STAT_OPTION,
    PRODUCER_STAT_OPTION,
    CONSUMER_STAT_OPTION,
    PRINT_CONSUMERS_MESSAGE_STAT,
    PRINT_PRODUCER_MESSAGE_STAT,
    Q_PLUS_PLUS_OPTION,
    Q_MINUS_MINUS_OPTON,
    Q_OPTION
}OPTIONS;

typedef struct {
    pthread_t tid;          //ID процесса
    int work_flag;
    int num_mess;
}thread;

int check_hash(struct message* msg);
void delete_all_and_exit();
void create_producer();
void create_consumer();
void print_menu();
OPTIONS get_option(char* buf, int num);
void delete_last_consumer();
void delete_last_producer();
void print_queue_stat(struct ring_buffer*);
void print_producers_stat();
void print_consumers_stat();
void resize_queue(int);
void print_consumers_message_stat();
void print_producers_message_stat();
void draw_histogram(thread* array ,int values[], int length);
void print_producers_message_stat();
void print_consumers_message_stat();
void incrementProducerMessage(pthread_t tid);
void incrementConsumerMessage(pthread_t tid);