#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <locale.h>
#define NAME_MAX 255

char currentNameChild[] = "child_00\0";
extern char **environ;
//printf("\033[1;36mКаталог %s, отказано в доступе\033[0m\n", directoryName);
int compareCollateC(const void *a, const void *b) {
    return strcoll(*(char**)a, *(char**)b);
}

// Функция для поиска значения по ключу в массиве строк
const char* findValueByKey(char** array, char* key) {
    // Проходим по массиву строк, пока не достигнем строки равной NULL
    for (int i = 0; array[i] != NULL; i++) {
        // Используем функцию strtok для разделения строки на ключ и значение
        char* token = strtok((char*)array[i], "=");
        if (token != NULL) {
            // Сравниваем ключ с заданным ключом, если они совпадают, возвращаем значение
            if (strcmp(token, key) == 0) {
                // Возвращаем значение (строка после знака "=")
                return strtok(NULL, "=");
            }
        }
    }
    // Если ключ не был найден, возвращаем NULL
    return NULL;
}

int getEnvSize(char* env[]){
    int envSize = 0;
    while (env[envSize] != NULL) {
        envSize++;
    }
    return envSize;
}

void incrementName(){
    int len = strlen(currentNameChild);
    if (currentNameChild[len-1]=='9'){
        currentNameChild[len-2]++;
        currentNameChild[len-1]='0';
    }
    else currentNameChild[len-1]++;
}

void printEnv(char* env[]){
    // Вывод отсортированных переменных окружения
    printf ("\033[1;36mEnviroment of parent:\n\033[0m");
    for (int i = 0; i < getEnvSize(env); i++) {
        printf("%s\n", env[i]);
    }
}

void runChild1(char* args[], char* env[]){
    int childStatus;
    printf("\033[1;34mChild with option '+' started...\033[0m\n");
    pid_t childPid = fork();
    if (childPid == -1) {
        printf("\033[1;32mError occured, error code - %d\n\033[0m", errno);
        exit(errno);
    }
    if (childPid==0){
        printf("\033[1;32m%s process created. Please, wait...\n\033[0m", currentNameChild);   
        execve(getenv("CHILD_PATH"),args,env);
    }
    waitpid(childPid, &childStatus, 0);
    printf("\033[1;32mChild process have ended with %d exit status\n\033[0m", childStatus);
}

void runChild2(char* args[], char* env[]){
    int childStatus;
    printf("\033[1;34mChild with option '*' started...\033[0m\n");
    pid_t childPid = fork();
    if (childPid == -1) {
        printf("Error occured, error code - %d\n", errno);
        exit(errno);
    }
    if (childPid==0){
        printf("\033[1;32m%s process created. Please, wait...\n\033[0m", currentNameChild);    
        execve(findValueByKey(env,"CHILD_PATH"),args,env);
    }
    waitpid(childPid, &childStatus, 0);
    printf("\033[1;32mChild process have ended with %d exit status\n\033[0m", childStatus);
}

void runChild3(char* args[], char* env[]){
    int childStatus;
    printf("\033[1;34mChild with option '&' started...\033[0m\n");
    pid_t childPid = fork();
    if (childPid == -1) {
        printf("Error occured, error code - %d\n", errno);
        exit(errno);
    }
    if (childPid==0){
        printf("\033[1;32m%s process created. Please, wait...\n\033[0m", currentNameChild);   
        execve(findValueByKey(environ,"CHILD_PATH"),args,env);
    }
    waitpid(childPid, &childStatus, 0);
    printf("\033[1;32mChild process have ended with %d exit status\n\033[0m", childStatus);
}

int main(int argc, char* argv[], char* env[]) {
    setlocale(LC_COLLATE,"C");
    
    if (argc!=2){
        printf ("Incorrect input, input name of file\n ");
        exit(2);
    }

    qsort(env, getEnvSize(env), sizeof(char*), compareCollateC);

    printEnv(env);
    
    char mode;
    char* args[] = {currentNameChild, argv[1], &mode, (char*)0};
    char c;
    int flag=1;
    

    do  {
        printf ("\033[1;33mInput panel:\n\033[0m");
        c = getchar();
        getchar();
        switch (c){
            case '+':
                args[2][0] = '+';
                runChild1(args, env);
                incrementName();
                break;
            case '*':
                args[2][0] = '*';
                runChild2(args, env);
                incrementName();
                break;
            case '&':
                args[2][0] = '&';
                runChild3(args, env);
                incrementName();
                break;
            case 'q':
                flag =0;
                break;
            default:
                printf ("Error input\n");
                break;
        }

    }while(flag);
    
}

