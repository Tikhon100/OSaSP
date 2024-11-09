
#include "parent_func.h"
extern int current_child_num;
extern child_struct* childs_array;
int g_func_flag=0;

int count_occurrences(char *str, char ch) {
    int count = 0;
    for (size_t i=0;i<strlen(str);i++){
        if (str[i]==ch){
            count++;
        }
    }
    return count;
}

int check_numbers(char* begin, char* end){
    if (end<begin){
        return 0;
    }
    for (char* i = begin; i<=end;i++){
        if ((*i>='0' && *i<'9')==0){
            return 0;
        }
    }
    return 1;
}


OPTIONS get_option(char* buf, int num){
    while (1){
        fgets(buf, num, stdin);

        if (buf[0]=='+' && strlen(buf)==2){
            return CREATE_OPTION;
        }
        else if (buf[0]=='-' && strlen(buf)==2){
            return DELETE_OPTION;
        }
        else if (buf[0]=='l' && strlen(buf)==2){
            return L_OPTION;
        }
        else if (buf[0]=='k' && strlen(buf)==2){
            return K_OPTION;
        }
        else if (buf[0]=='s' && strlen(buf)==2){
            return S_OPTION;
        }
        else if (buf[0]=='g' && strlen(buf)==2){
            return G_OPTION;
        }
        else if (buf[0]=='s' && strstr(buf, "<")!=NULL && strstr(buf,">")!=NULL && buf[1]=='<' && buf[strlen(buf)-2]=='>' && 
            count_occurrences(buf, '<')==1 && count_occurrences(buf, '>')==1 && strstr(buf,"<")<strstr(buf,">")){            
            if (check_numbers(strstr(buf,"<")+1, strstr(buf,">")-1)){
                return S_NUM_OPTION;
            }  
        }
        else if (buf[0]=='g' && strstr(buf, "<")!=NULL && strstr(buf,">")!=NULL && buf[1]=='<' && buf[strlen(buf)-2]=='>' &&
            count_occurrences(buf, '<')==1 && count_occurrences(buf, '>')==1 && strstr(buf,"<")<strstr(buf,">")){
            if (check_numbers(strstr(buf,"<")+1, strstr(buf,">")-1)){
                return G_NUM_OPTION;
            }  
        }
        else if (buf[0]=='p' && strstr(buf, "<")!=NULL && strstr(buf,">")!=NULL && buf[1]=='<' && buf[strlen(buf)-2]=='>' &&
            count_occurrences(buf, '<')==1 && count_occurrences(buf, '>')==1 && strstr(buf,"<")<strstr(buf,">")){
            if (check_numbers(strstr(buf,"<")+1, strstr(buf,">")-1)){
                return P_NUM_OPTION;
            }  
        }
        else if (buf[0]=='q' && strlen(buf)==2){
            return Q_OPTION;
        }
        printf ("Error input - try again:\n");
    }
}
void create_child(){
    //ls -l /proc/<pid>/fd/0 - в ответ придет строка, которую указываем в open
    int new_terminal = open("/dev/pts/1", O_RDWR);
    
    pid_t temp_pid = fork();      
    current_child_num++; 
    childs_array = (child_struct*)realloc(childs_array, current_child_num * sizeof(child_struct));  

    childs_array[current_child_num-1].is_stoped = 0;
    sprintf(childs_array[current_child_num-1].name, "C_%d", current_child_num);
    childs_array[current_child_num-1].pid = temp_pid;

    if (temp_pid < 0) {
        fprintf(stderr,"%s\n" ,strerror(errno));
        exit(1);
    } else if (temp_pid == 0) {
        if (new_terminal!=-1){
            dup2(new_terminal, STDIN_FILENO);
            dup2(new_terminal, STDOUT_FILENO);
            dup2(new_terminal, STDERR_FILENO);
        }
        execl("./child.exe", childs_array[current_child_num-1].name, NULL);
    } else {
        printf("PID %d(%s) - created\n", temp_pid, childs_array[current_child_num-1].name);
        printf("count of children proceses - %d\n", current_child_num);
    }
}

void delete_last_child(){
    if (current_child_num>0){
        kill(childs_array[current_child_num-1].pid,SIGKILL);
        printf ("Child %s deleted\n",childs_array[current_child_num-1].name);
        current_child_num--;
        childs_array = (child_struct*)realloc(childs_array, current_child_num*sizeof(child_struct));
    }
    else {
        printf("There are no childs to delete\n");
    }
}

void print_all(char* parent_name){
    printf ("Parent process:\n");
    printf ("Name: %s, ppid: %d, pid: %d\n", parent_name, getppid(), getpid());
    printf ("Childs processes:\n");
    if (current_child_num>0){
        for (int i=0;i<current_child_num;i++){
            printf ("Name: %s, ppid: %d, pid: %d, stat: %d\n", childs_array[i].name, getpid(), childs_array[i].pid, childs_array[i].is_stoped);
        }
    }
    else {
        printf ("There are no childs\n");
    }
}

void delete_all(){
    if (current_child_num==0){
        printf("There are no childs to delete\n");
    }
    else {
        for (int i=0;i<current_child_num;i++){
            kill(childs_array[i].pid,SIGKILL);
            printf ("Child %s deleted\n",childs_array[i].name);
        }
        current_child_num=0;
        free(childs_array);
        printf ("All childs deleted:\n");
    }
}

void ban_all_stat(){
    if (current_child_num==0){
        printf("There are no childs to ban stat\n");
    }
    else {
        for (int i=0;i<current_child_num;i++){
            kill(childs_array[i].pid, SIGUSR1);
        }
        printf ("All childs stat stoped\n");
    }
}
void allow_all_stat(){
    if (current_child_num==0){
        printf ("There are no chilids to alow stat\n");
    }
    else {
        for (int i=0;i<current_child_num;i++){
            kill(childs_array[i].pid, SIGUSR2);
        }
        printf ("All childs stat allowed\n");
    }
    g_func_flag=1;
}
int read_number_from_str(char* start, char* end) {
    int result = 0;
    for (char* i = start; i <= end; i++) {

        if (*i >= '0' && *i <= '9') {
            result = result * 10 + (*i - '0');
        } else {
            return -1;
        }
    }
    return result;
}
void ban_stat(int num){
    if (num==-1){
        printf ("Incorret num -1\n");
    }
    else if (num>current_child_num || num==0){
        printf ("There are no child with such number\n");
    }
    else {
        printf ("Stopped C_%d\n", num);
        kill(childs_array[num-1].pid, SIGUSR1);
    }
}
void allow_stat(int num){
    if (num==-1){
        printf ("Incorret num -1\n");
    }
    else if (num>current_child_num || num==0){
        printf ("There are no child with such number\n");
    }
    else {
        printf ("Allowed C_%d\n", num);
        kill(childs_array[num-1].pid, SIGUSR2);
    }
}

void alarm_hand(int sig){
    if (g_func_flag==0){
        printf ("G option not sended for %d seconds, all stat allowed\n", P_OPTION_DELAY_TIME);
        allow_all_stat();
    }
}

void request_stat(int num){
    if (num==-1){
        printf ("Incorret num -1\n");
    }
    else if (num>current_child_num || num==0){
        printf ("There are no child with such number\n");
    }
    else {
        kill(childs_array[num-1].pid, SIGURG);
    }
    g_func_flag=0;
    signal(SIGALRM, alarm_hand);
    alarm(P_OPTION_DELAY_TIME);
}