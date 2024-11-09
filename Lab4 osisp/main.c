
#include "general_func.h"
#include "main_func.h"

procces* consumers_array;
procces* producers_array;
int current_num_producers =0;
int current_num_consumers =0;
struct message null_message;

void sigint_handler(int signal){
    delete_all_and_exit();
}

int main() {
    srand(time(NULL));
    signal(SIGINT, sigint_handler);

    struct message null_message = create_null_message();
    key_t key = ftok("ftokfile.txt", 'A'); // генерируем ключ для сегмента общей памяти
    key_t key_sem = key + 1;
    key_t key_produser = key + 2;
    key_t key_consumer = key + 3;

    int is_blocked_sem = semget(key_sem, 1, IPC_CREAT | 0666);      //сохдаём семафоры
    int full = semget(key_produser, 1, IPC_CREAT | 0666);
    int empty = semget(key_consumer, 1, IPC_CREAT | 0666); 
    
    
    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    } arg;

    arg.val = 1;            //инициализация одноместного семафора для доступа к буферу
    if (semctl(is_blocked_sem, 0, SETVAL, arg) == -1) {
        perror("semctl");
        exit(1);
    }
    arg.val = NUM_MESSAGES-3;           //инициализация семафора для отслеживания свободных мест в буфере
    if (semctl(empty, 0, SETVAL, arg) == -1) {
        perror("semctl");
        exit(1);
    }
    arg.val = 3;            //инициализация семафора для отслеживания сообщения в буфере
    if (semctl(full, 0, SETVAL, arg) == -1) {
        perror("semctl");
        exit(1);
    }
    
    
    int shmid = shmget(key, sizeof(struct ring_buffer)+256*NUM_MESSAGES, IPC_CREAT | 0666); // создаем сегмент общей памяти
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }
    struct ring_buffer *shared_buffer = (struct ring_buffer *)shmat(shmid, NULL, 0); // присоединяем сегмент к адресному пространству процесса
    if (shared_buffer == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    init_buffer(shared_buffer);
    add_message(shared_buffer, generateMessage());
    add_message(shared_buffer, generateMessage());
    add_message(shared_buffer, generateMessage());

    printf("\n%s>---------MAIN--------<\n", GREEN);
    
    for (int i=0;i<3;i++){

        printf ("%sType:%s %d\n",GREEN,RESET, shared_buffer->messages[i].type);
        printf ("%sHash%s: %d\n",GREEN,RESET, shared_buffer->messages[i].hash);
        printf ("%sSize:%s %d\n",GREEN,RESET, shared_buffer->messages[i].size);
        printf ("%sData: %s",GREEN,RESET);
        for (int j=0;j<shared_buffer->messages[i].size;j++){
            printf ("%c", shared_buffer->messages[i].data[j]);
        }
        printf ("\n");
    }
    printf("%s>---------MAIN---------<%s\n\n",GREEN,RESET);

    print_menu();

    char* buf = (char*)malloc(sizeof(char)*10);
    while(1){
        OPTIONS num = get_option(buf,10);
        switch (num){
            case CREATE_PRODUCER_OPTION:
                create_producer();
                break;
            case CREATE_CONSUMER_OPTION:
                create_consumer();
                break;
            case DELETE_CONSUMER_OPTION:
                delete_last_consumer();
                break;
            case DELETE_PRODUCER_OPTION:
                delete_last_producer();
                break;
            case QUEUE_STAT_OPTION:
                print_queue_stat(shared_buffer);
                break;   
            case PRODUCER_STAT_OPTION:
                print_producers_stat();
                break;     
            case CONSUMER_STAT_OPTION:
                print_consumers_stat();
                break;
            case Q_OPTION:
                delete_all_and_exit();
                break;          
        }        
    }   


    if (semctl(is_blocked_sem, 0, IPC_RMID, 0) == -1) {             //удаления семафоров
        perror("Ошибка удаления набора семафора is_blocked_sem");
        exit(EXIT_FAILURE);
    }
    if (semctl(full, 0, IPC_RMID, 0) == -1) {
        perror("Ошибка удаления набора семафора full");
        exit(EXIT_FAILURE);
    }
    if (semctl(empty, 0, IPC_RMID, 0) == -1) {
        perror("Ошибка удаления набора семафора empty");
        exit(EXIT_FAILURE);
    }

    shmdt(shared_buffer); // отсоединяем сегмент от адресного пространства процесса
    shmctl(shmid, IPC_RMID, NULL); // удаляем сегмент общей памяти после использования

    return 0;
}