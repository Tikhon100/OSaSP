#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

extern char **environ;

char* cutStr(char* str, char c){
    for (int i=0;;i++){
        if (str[i]==c){
            char* ret = (char*)malloc((i+1)*sizeof(char));
            int j=0;
            for (j = 0;j<i;j++){
                ret[j]=str[j];
            }
            ret[j]='\0';
            return ret;
        }
        if (str[i]=='\0'){
            return NULL;
        }
    }
    return NULL;
}

char* strAfterSym(char* str, char c){
    for (int i=0;;i++){
        if (str[i]==c){
            return str+i+1;
        }
    }
}

// Функция для поиска значения по ключу в массиве строк
const char* findValueByKey(char** array, char* key) {
    for (int i = 0; array[i] != NULL; i++) {

        char* token = cutStr(array[i], '=');
        if (token != NULL) {
            if (strcmp(token, key) == 0) {
                char* value = strAfterSym(array[i], '=');
                return value;
            }
        }
    }
    return NULL;
}

int main(int argc, char *argv[], char* env[]) {


    fprintf(stdout, "%s process begins...\n", argv[0]);
    if (argc!=3){
        printf("Error in child Propram params!");
        exit (120);
    }
    char mode = argv[2][0];
    pid_t pid = getpid();
    pid_t ppid = getppid();
    FILE *File = fopen(argv[1],"r");
    char variable[256];

    printf ("\033[0m________________________\n");
    printf ("|  Hello, i am child!  |\n");
    printf ("|                      |\n");
    printf ("|My pid : %5d        |\n", (int)pid);
    printf ("|My pid : %5d        |\n", (int)ppid);
    printf ("|My name: %s     |\n", argv[0]);
    printf ("________________________\n");
    
    
    if (File == NULL) {
        printf("File %s not open\n", argv[1]);
        exit(-1);
    }

    while (fgets(variable, 255, File) != NULL) {  
        if (variable[strlen(variable)-1]=='\n'){
            variable[strlen(variable)-1] = '\0';
        }

        switch (mode){
            case '+':
                printf("%s=%s\n", variable, getenv(variable));
                break;
            case '*':
                printf("%s=%s\n", variable, findValueByKey(env,variable));
                break;
            case '&':
                printf("%s=%s\n", variable, findValueByKey(environ,variable));
                break;
        }    
    }    
    fclose(File);
    exit(0);
}