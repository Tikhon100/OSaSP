
#include "main_func.h"
extern thread* consumers_array;
extern thread* producers_array;
extern int current_num_producers;
extern int current_num_consumers;
extern int NUM_MESSAGES;
extern struct ring_buffer* shared_buffer;
extern pthread_mutex_t mutex;
extern pthread_cond_t full;
extern pthread_cond_t empty;
void* producer_thread(void* arg) {
    
    while (getProducerFlag(pthread_self()) == 0 ) {
        pthread_mutex_lock(&mutex);
        while (check_buffer(shared_buffer) == 1 && getProducerFlag(pthread_self()) == 0 ) {
            printf("\n%s<---------PRODUCER %ld--------->%s\n", YELLOW, pthread_self(), RESET);
            printf("Buffer is full\n");
            printf("%s<---------PRODUCER %ld--------->%s\n\n", YELLOW, pthread_self(), RESET);
            pthread_cond_wait(&full, &mutex);
        }
        struct message msg = generateMessage();
        int n = add_message(shared_buffer, msg);

        if (n != -1) {
            printf("%s\n<---------PRODUCER %ld--------->\n%s", YELLOW, pthread_self(), RESET);

            printf("%stype -%s %d\n", YELLOW, RESET, msg.type);
            printf("%shash -%s %d\n", YELLOW, RESET, msg.hash);
            printf("%ssize -%s %d\n", YELLOW, RESET, msg.size);
            printf("%sdata - %s", YELLOW, RESET);
            for (int i = 0; i < msg.size; i++) {
                printf("%c", msg.data[i]);
            }
            printf("%s\n<---------PRODUCER %ld--------->\n\n%s", YELLOW, pthread_self(), RESET);
            incrementProducerMessage(pthread_self());
        }
        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&mutex);

        sleep(3);
    }
    
    pthread_exit(NULL);
}

void* consumer_thread(void* arg) {
    
    while (getConsumerFlag(pthread_self())==0) {
        pthread_mutex_lock(&mutex);
        while (check_buffer(shared_buffer) == -1 && getConsumerFlag(pthread_self())==0) {
            printf("\n%s>---------CONSUMER %ld---------<%s\n", RED, pthread_self(), RESET);
            printf("Buffer is empty");
            printf("\n%s>---------CONSUMER %ld---------<%s\n\n", RED, pthread_self(), RESET);
            pthread_cond_wait(&empty, &mutex);
        }
        if (shared_buffer->count_added - shared_buffer->count_extracted > 0){
            struct message msg = extract_message(shared_buffer);
            if (!check_hash(&msg)) {
                printf("Error in message!\n");
            } else {
                printf("\n%s>---------CONSUMER %ld---------<%s\n", RED,pthread_self(), RESET);
                printf("%stype -%s %d\n", RED, RESET, msg.type);
                printf("%shash -%s %d\n", RED, RESET, msg.hash);
                printf("%ssize -%s %d\n", RED, RESET, msg.size);
                printf("%sdata -%s ", RED, RESET);
                for (int i = 0; i < msg.size; i++) {
                   printf("%c", msg.data[i]);
                }
                printf("\n%s>---------CONSUMER %ld---------<%s\n\n", RED, pthread_self(), RESET);
            }
            incrementConsumerMessage(pthread_self());
        }
        
        pthread_cond_signal(&full);
        pthread_mutex_unlock(&mutex);

        sleep(3);
    }
    
    pthread_exit(NULL);
}

void create_producer(){   
    current_num_producers++;
    producers_array = (thread*)realloc(producers_array, current_num_producers * sizeof(thread));  

    atomic_store(&producers_array[current_num_producers-1].should_exit, 0);
    producers_array[current_num_producers-1].num_mess = 0;
    pthread_t producer_thread_id;
    pthread_create(&producer_thread_id, NULL, producer_thread, NULL);
    producers_array[current_num_producers-1].tid = producer_thread_id;
    
    printf ("Thread with tid %ld created\n", producers_array[current_num_producers-1].tid);
}
void create_consumer(){
    
    current_num_consumers++;
    consumers_array = (thread*)realloc(consumers_array, current_num_consumers * sizeof(thread));  
    pthread_cond_broadcast(&empty);
    atomic_store (&consumers_array[current_num_consumers-1].should_exit, 0);
    consumers_array[current_num_consumers-1].num_mess = 0;
    pthread_t consumer_thread_id;
    pthread_create(&consumer_thread_id, NULL, consumer_thread, NULL);
    consumers_array[current_num_consumers-1].tid = consumer_thread_id;
    
    printf("Thread with tid %ld created\n", consumers_array[current_num_consumers-1].tid);

}

void delete_last_consumer(){
    if (current_num_consumers>0){
        atomic_store(&consumers_array[current_num_consumers-1].should_exit, 1);
    
        pthread_cond_broadcast(&empty);
        
        //pthread_cancel(consumers_array[current_num_consumers-1].tid);
        pthread_join(consumers_array[current_num_consumers-1].tid, NULL);
        printf ("Consumer %ld deleted\n",consumers_array[current_num_consumers-1].tid);
        current_num_consumers--;
        consumers_array = (thread*)realloc(consumers_array, current_num_consumers*sizeof(thread));
    
    }
    else {
        printf("There are no consumers to delete\n");
    }
    
}
void delete_last_producer() {
    if (current_num_producers > 0) {
        atomic_store(&producers_array[current_num_producers-1].should_exit, 1);
        pthread_cond_broadcast(&full);
        pthread_join(producers_array[current_num_producers-1].tid, NULL);
        printf("Producer %ld deleted\n", producers_array[current_num_producers - 1].tid);
        current_num_producers--;
        producers_array = (thread*)realloc(producers_array, current_num_producers * sizeof(thread));
        
    } else {
        printf("There are no producers to delete\n");
    }
}

int check_hash(struct message* msg){
    return msg->hash==calculateHash(msg->data, msg->size);
}

void print_menu(){
    printf ("Menu:\n");
    printf ("+p - create producer\n");
    printf ("-p - create producer\n");
    printf ("+c - create consumer\n");
    printf ("-c - create consumer\n");
    printf ("ps - print producer stat\n");
    printf ("cs - print consumer stat\n");
    printf ("qs - print queue stat\n");
    printf ("p - print producers histogram\n");
    printf ("c - print consumers histogram\n");
    printf ("q+ - increase queue size\n");
    printf ("q- - decrease queue size\n");
    printf ("q - exit\n");
}

void delete_all_and_exit(){
    while(current_num_consumers>0){
        delete_last_consumer();
    }
    while(current_num_producers>0){
        delete_last_producer();
    }
    exit(0);
}

int getProducerFlag(pthread_t tid){
    for (int i = 0; i<current_num_producers;i++){
        if (pthread_equal(producers_array[i].tid,tid)){
            return (int)atomic_load(&producers_array[i].should_exit);
        }
    }
    return 1;
}
int getConsumerFlag(pthread_t tid){
    for (int i=0;i<current_num_consumers;i++){
        if (pthread_equal(consumers_array[i].tid,tid)){
            return (int)atomic_load(&consumers_array[i].should_exit);
        }
    }
    return 1;
}

void incrementProducerMessage(pthread_t tid){
    for (int i = 0; i<current_num_producers;i++){
        if (pthread_equal(producers_array[i].tid,tid)){
            producers_array[i].num_mess++;
        }
    }
}

void incrementConsumerMessage(pthread_t tid){
    for (int i = 0; i<current_num_consumers;i++){
        if (pthread_equal(consumers_array[i].tid,tid)){
            consumers_array[i].num_mess++;
        }
    }
}

OPTIONS get_option(char* buf, int num){
    while (1){
        fgets(buf, num, stdin);

        if (buf[0]=='+' && buf [1]=='p' && strlen(buf)==3){
            return CREATE_PRODUCER_OPTION;
        }
        else if (buf[0]=='-' && buf[1]=='p' && strlen(buf)==3){
            return DELETE_PRODUCER_OPTION;
        }
        else if (buf[0]=='+' && buf [1]=='c' && strlen(buf)==3){
            return CREATE_CONSUMER_OPTION;
        }
        else if (buf[0]=='-' && buf[1]=='c' && strlen(buf)==3){
            return DELETE_CONSUMER_OPTION;
        }
        else if (buf[0]=='q' && buf[1]=='s' && strlen(buf)==3){
            return QUEUE_STAT_OPTION;
        }
        else if (buf[0]=='p' && buf[1] == 's' && strlen(buf)==3){
            return PRODUCER_STAT_OPTION;
        }
        else if (buf[0]=='c' && buf[1] == 's' && strlen(buf)==3){
            return CONSUMER_STAT_OPTION;
        }
        else if (buf[0]=='q' && strlen(buf)==2){
            return Q_OPTION;
        }
        else if (buf[0]=='q' && buf[1] == '+' && strlen(buf)==3){
            return Q_PLUS_PLUS_OPTION;
        }
        else if (buf[0]=='q' && buf[1] == '-' && strlen(buf)==3){
            return Q_MINUS_MINUS_OPTON;
        }
        else if (buf[0] == 'c' && strlen(buf)==2){
            return PRINT_CONSUMERS_MESSAGE_STAT;
        }
        else if (buf [0] == 'p' && strlen(buf) ==2){
            return PRINT_PRODUCER_MESSAGE_STAT;
        }
        printf ("Error input - try again:\n");
    }
}

void resize_queue(int flag){
    if (NUM_MESSAGES==1 && flag==-1){
        printf ("NUM_MESSAGES is 1, can`t degrease\n");
        return;
    }

    if (flag==1){
        struct message* new_messages = (struct message*)malloc(sizeof(struct message)*(NUM_MESSAGES+1));
        pthread_mutex_lock(&mutex);
        int current_num_in_queue = shared_buffer->count_added-shared_buffer->count_extracted;

        for (int i=0,j=shared_buffer->head;i<current_num_in_queue;i++){
            new_messages[i] = shared_buffer->messages[j];      
            j++;
            if (j==NUM_MESSAGES){
                j=0;
            }
        }
        free(shared_buffer->messages);
        shared_buffer -> messages = new_messages;
        shared_buffer->head = 0;
        shared_buffer->tail = current_num_in_queue;
        NUM_MESSAGES++;  
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&full);
    }
    else if (flag==-1){
        struct message* new_messages = (struct message*)malloc(sizeof(struct message)*(NUM_MESSAGES-1));
        pthread_mutex_lock(&mutex);
        int current_num_in_queue = shared_buffer->count_added-shared_buffer->count_extracted;
        int full_flag=0;
        if (current_num_in_queue==NUM_MESSAGES){
            printf("%sWasted last message%s\n",RED,RESET);
            full_flag=1;
        }
        for (int i=0,j=shared_buffer->head;i<current_num_in_queue-full_flag;i++){
            new_messages[i] = shared_buffer->messages[j];      
            j++;
            if (j==NUM_MESSAGES){
                j=0;
            }
        }
        free(shared_buffer->messages);
        shared_buffer -> messages = new_messages;
        shared_buffer->head = 0;
        shared_buffer->tail = current_num_in_queue-full_flag;
        shared_buffer->count_added-=full_flag;
        NUM_MESSAGES--;
        pthread_mutex_unlock(&mutex);
    }  
    printf("New size of queue %d\n", NUM_MESSAGES);
}



void print_queue_stat(struct ring_buffer* buffer){
    printf ("Stat of queue:\n");
    int num_in_queue  = buffer->count_added-buffer->count_extracted;
    printf ("Num elements in queue: %d\n", num_in_queue);
    printf ("Num of free slots %d\n", NUM_MESSAGES-num_in_queue);
    printf ("Count_added - %d\n", buffer->count_added);
    printf ("Count_extrected - %d\n", buffer->count_extracted);
    int n = buffer->count_added - buffer->count_extracted;
    for (int i=0,j=buffer->head;i<n;i++){
        printf ("Message %d\n", i+1);
        printf ("   Type: %d\n", buffer->messages[j].type);
        printf ("   Hash: %d\n", buffer->messages[j].hash);
        printf ("   Size: %d\n   Data: ", buffer->messages[j].size);
        for (int k=0;k<buffer->messages[j].size;k++){
            printf ("%c", buffer->messages[j].data[k]);
        }
        printf ("\n");
        j++;
        if (j==NUM_MESSAGES){
            j=0;
        }
    }
}
void print_producers_stat(){
    printf ("Producers processes:\n");
    if (current_num_producers>0){
        for (int i=0;i<current_num_producers;i++){
            printf ("Tid: %ld\n", producers_array[i].tid);
        }
    }
    else {
        printf ("There are no producers\n");
    }
}
void print_consumers_stat(){
    printf("Consumers processes:\n");
    if (current_num_consumers > 0) {
        for (int i = 0; i < current_num_consumers; i++) {
            printf ("Tid: %ld\n", consumers_array[i].tid);
        }
    } else {
     printf("There are no consumers\n");
    }
}

void print_consumers_message_stat(){
    int * array = (int*)malloc(sizeof(int) * current_num_consumers);
    for (int i=0;i<current_num_consumers;i++){
        array[i] = consumers_array[i].num_mess;
    }
    draw_histogram(consumers_array, array, current_num_consumers);
    free(array);
}

void print_producers_message_stat(){
    int *array = (int*)malloc(sizeof(int)* current_num_producers);
    for (int i=0;i<current_num_producers;i++){
        array[i] = producers_array[i].num_mess;
    }
    draw_histogram(producers_array, array, current_num_producers);
    free(array);
}

void draw_histogram(thread* array ,int values[], int length) {
    int i, j;
    int max_value = values[0];  // Максимальное значение в массиве
    char bar_char = '#';  // Символ, используемый для отображения столбцов

    // Находим максимальное значение в массиве
    for (i = 1; i < length; i++) {
        if (values[i] > max_value) {
            max_value = values[i];
        }
    }

    // Отображаем диаграмму
    for (i = 0; i < length; i++) {
        int normalized_value = (int)((double)values[i] / max_value * 50);  // Нормализация значений для ширины диаграммы
        printf ("%ld value: ",array[i].tid);
        printf("%3d: ", values[i]);
        for (j = 0; j < normalized_value; j++) {
            putchar(bar_char);
        }
        putchar('\n');
    }
}