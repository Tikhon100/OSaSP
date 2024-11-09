
#define _GNU_SOURCE

#include "parent_func.h"
#include <stdio.h>

int current_child_num=0;
child_struct* childs_array = NULL; 

void sigint_handler(int signum) {
    printf("\nReceived SIGINT (Ctrl+C)\nKill all childs. . .\n");
    delete_all();
    printf ("Exit from parent prog\n");
    exit(0);
}

int main (int argc, char* argv[]){
    signal(SIGINT, sigint_handler);

    char* buf = (char*)malloc(sizeof(char)*10);
        while(1){
            OPTIONS num = get_option(buf,10);
            switch (num){
                case CREATE_OPTION:
                    create_child();
                    break;
                case DELETE_OPTION:
                    delete_last_child();
                    break;
                case L_OPTION:
                    print_all(argv[0]);
                    break;  
                case K_OPTION:
                    delete_all();
                    break;  
                case S_OPTION:
                    ban_all_stat();
                    break;  
                case G_OPTION:
                    allow_all_stat();
                    break;        
                case S_NUM_OPTION:
                    ban_stat(read_number_from_str(strstr(buf,"<")+1,strstr(buf,">")-1));
                    break;  
                case G_NUM_OPTION:
                    allow_stat(read_number_from_str(strstr(buf,"<")+1,strstr(buf,">")-1));
                    break;  
                case P_NUM_OPTION:
                    ban_all_stat();
                    request_stat(read_number_from_str(strstr(buf,"<")+1,strstr(buf,">")-1));
                    break;  
                case Q_OPTION:
                    delete_all();
                    printf ("Exit from parent prog. . . \n");
                    return 0;
                    break;      
            }        
        }    

    return 0;    
}