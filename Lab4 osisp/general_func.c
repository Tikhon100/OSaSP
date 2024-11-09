#include "general_func.h"

unsigned char calculateHash(unsigned char *data, unsigned char size) {
    unsigned int hash = 2166136261;  // Начальное значение хеша
    for (unsigned int i = 0; i < size; i++) {
        hash ^= data[i];
        hash *= 16777619;            // Простое число для хеширования
    }
    return hash;
}

int check_buffer(struct ring_buffer* buffer){
    int num_in_queue = buffer->count_added-buffer->count_extracted;
    if (num_in_queue==0){
        return -1; // очередь пуста
    }
    if (num_in_queue>=NUM_MESSAGES){
        return 1;  // очередь полная
    }
    return 0;
}

char generateRandomLetter() {
    char letter;
    int randomCase = rand() % 2; // Генерация случайного числа 0 или 1
    
    if (randomCase == 0) {
        letter = 'a' + rand() % 26; // Случайная строчная буква
    } else {
        letter = 'A' + rand() % 26; // Случайная заглавная буква
    }
    
    return letter;
}

struct message generateMessage() {
    struct message msg;
    
    msg.type = rand() % 2; 
    
    
    int size = rand() % 257;
    while(size == 0){
        size = rand() % 257;
    }
    if (size == 256){
        size = 0;
    }
    msg.size = size;
    
    int i;
    for ( i = 0; i < size-1; i++) {
        msg.data[i] = generateRandomLetter(); // Генерация случайного байта данных
    }
    msg.data[i] = '\0';
    
    
    msg.hash = calculateHash(msg.data, size);
    
    return msg;
}


void init_buffer(struct ring_buffer *buffer){
    buffer->head = 0;
    buffer->tail = 0;
    buffer->count_added = 0;
    buffer->count_extracted = 0;
}

struct message create_null_message(){
    struct message msg;
    msg.hash=0;
    msg.size=0;
    msg.type=0;
    for (int i=0;i<256;i++){
        msg.data[i]=0;
    }
    return msg;
}

void set_sops(struct sembuf *sops,int a, int b, int c){
    sops->sem_num = a;
    sops->sem_op = b;
    sops->sem_flg = c;
}

int add_message(struct ring_buffer* buffer, struct message msg) {
    if (buffer->count_added-buffer->count_extracted>=NUM_MESSAGES){ // места нет
        return -1;
    }
    else {
        buffer->count_added++;
        buffer->messages[buffer->tail]=msg;
        buffer->tail++;
        if (buffer->tail == NUM_MESSAGES){
            buffer->tail = 0;
        } 
        return 0;
    }
}

struct message extract_message(struct ring_buffer* buffer) {
    struct message null_message = create_null_message();

    if (buffer->count_added - buffer->count_extracted > 0) { // есть сообщения в буфере
        struct message msg = buffer->messages[buffer->head];
        buffer->messages[buffer->head] = null_message;
        buffer->count_extracted++;
        buffer->head++;
        if (buffer->head==NUM_MESSAGES){
            buffer->head=0;
        }

        return msg;
    } else {
        return null_message;
    }
}
