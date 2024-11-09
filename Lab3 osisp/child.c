
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
int num_loops = 10;
int stat_stopped = 0;
int num_iterations = 0;
char* argv_0_ptr=NULL;

typedef struct {
    int num_00;
    int num_01;
    int num_10;
    int num_11;
} statistic;

statistic st;

typedef struct{
    int a;
    int b;
}int_pair;
int_pair pair;


void change_pair(){
    usleep(80000);
    if (pair.a==0){
        pair.a=1;
        usleep(60000 + rand()%40000);
        pair.b=1;
    }
    else {
        pair.a=0;
        usleep(60000 + rand()%40000);
        pair.b=0;
    }
}

void alarm_handler(int sig) {  
    if (pair.a==0 && pair.b==0){
        st.num_00++;
    }
    else if (pair.a==0 && pair.b==1){
        st.num_01++;
    }
    else if (pair.a==1 && pair.b==0){
        st.num_10++;
    }
    else if (pair.a==1 && pair.b==1){
        st.num_11++;
    }
    signal(SIGALRM, alarm_handler);     //инициализация обработчика будильника
    alarm(1+rand()%1);                //будильник от 0 до 4
}

void ban_stat(int sig){
    printf ("Thats child %s, my stat stopped\n", argv_0_ptr);
    stat_stopped=1;
    signal(SIGUSR1, ban_stat);
}

void allow_stat(int sig){
    printf ("Thats child %s, my stat allowed\n", argv_0_ptr);
    stat_stopped=0;
    signal(SIGUSR2, allow_stat);
}

void output_stat(int sig){
    printf ("Request of %s: ppid = %d, pid = %d, stat = {%d, %d, %d, %d}\n", argv_0_ptr,getppid(),getpid(),st.num_00,st.num_01,st.num_10,st.num_11);
    signal(SIGURG, output_stat);
}

int main(int argc, char* argv[]){
    argv_0_ptr = argv[0];

    st.num_00=0;
    st.num_01=0;
    st.num_10=0;
    st.num_11=0;

    srand(time(NULL));
    signal(SIGALRM, alarm_handler);     //инициализация обработчика будильника
    signal(SIGUSR1, ban_stat);
    signal(SIGUSR2, allow_stat);
    signal(SIGURG, output_stat);
    alarm(1+rand()%1);                

    while(1){
        if(num_iterations % num_loops == 0 && stat_stopped==0){
            printf ("%s: ppid = %d, pid = %d, stat = {%d, %d, %d, %d}\n", argv[0],getppid(),getpid(),st.num_00,st.num_01,st.num_10,st.num_11);
        }
        num_iterations++;
        change_pair();
    }

    return 0;
}

