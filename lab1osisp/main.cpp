#define DEFAULT_SOURSE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
int arrayofFlags [] = {0,0,0,0}; // l,d,f,s
int bigFlag =1;
char dirTypeArray [] = "Type: dir, name: ";
char lnkTypeArray [] = "Type: lnk, name: ";
char filTypeArray [] = "Type: fil, name: ";
// Функция сравнения для qsort
int compare_strings(const void *a, const void *b) {
	const char* aa = *(const char**)a;
 	const char* bb = *(const char**)b; 
    return strcmp(aa+17, bb+17);
}

int strLen(char* str){
	int i=0;    // без учета /0
	for (i;str[i]!='\0';i++){
	}
	return i;
}

void strCopy(char* str1, char* str2){ // copy str2 into str1
	int i=0;
	for (i=0;i<strLen(str2);i++){
		str1[i] = str2[i];
	}
	str1[i]='\0';
}

void modePrintf(char* str){
	printf("\033[0;32m"); // устанавливаем зеленый цвет текста
    for (int i = 0; i < 17; i++) {
        putchar(str[i]); // печатаем первые 17 символов
    }
    printf("\033[0m"); // сбрасываем цвет обратно в белый
    printf("%s\n", str + 17); // печатаем остальные символы
}

void dirwalk (char* directoryName){
	DIR* dir;
	struct dirent* currentDirent;

	char ** arrayOfFiles=(char**)malloc(0);
	int numElementsInArray=0;

	dir = opendir(directoryName); //открытие каталога
	//тут проверка
	if (!dir){
		if (errno == EACCES){
			printf("\033[1;36mКаталог %s, отказано в доступе\033[0m\n", directoryName);
			return;
		}
		else if (errno==ENOTDIR){
			printf("Invalid path\n");
		}
		else if (errno==ENOENT){
			printf("Directory does not exist\n");
		}
		printf("\033[1;37mНеизвестная ошибка\033[0m\n");
		return;
	}
	
	while ((currentDirent = readdir(dir)) != NULL) {

        if (!strcmp(".", currentDirent->d_name) || !strcmp("..", currentDirent->d_name)){
            continue;
		}

		int flag = (!arrayofFlags[0] && !arrayofFlags[1] && !arrayofFlags[2]) || (arrayofFlags[0] && currentDirent->d_type==DT_LNK) ||
		(arrayofFlags[1] && currentDirent->d_type==DT_DIR) || (arrayofFlags[2] && currentDirent->d_type == DT_REG);
		
		if (flag){
			arrayOfFiles = (char**)realloc(arrayOfFiles, sizeof(char*)*(numElementsInArray+1));
			numElementsInArray++;
			arrayOfFiles[numElementsInArray-1] = (char*)malloc(strLen(currentDirent->d_name)+1+17);
			char* typeArray;
			if (currentDirent->d_type==DT_DIR){
				typeArray = dirTypeArray;
			}
			else if (currentDirent->d_type==DT_LNK){
				typeArray = lnkTypeArray;
			}
			else typeArray = filTypeArray;
			for (int i=0;i<17;i++){
				arrayOfFiles[numElementsInArray-1][i] = typeArray[i];
			}
			strCopy((arrayOfFiles[numElementsInArray-1])+17,currentDirent->d_name);			
		}
        
		if (currentDirent->d_type==4){
			
			int len1 = strlen(directoryName);
    		int len2 = strlen(currentDirent->d_name);
			char* result = (char*)malloc((len1 + len2 + 2) * sizeof(char));
    
    		strcpy(result, directoryName);
			strcat(result, "/");
    		strcat(result, currentDirent->d_name);
			dirwalk (result);
			free (result);
		}

		
    }

	if (arrayofFlags[3]){
		qsort(arrayOfFiles, numElementsInArray, sizeof(char *), compare_strings);
	}
	printf("\033[1;33mСодержимое каталога: %s\033[0m\n", directoryName);
	for (int i=0;i<numElementsInArray;i++){
		modePrintf(arrayOfFiles[i]);
		free(arrayOfFiles[i]);
	}
	free(arrayOfFiles);
	closedir(dir);
}


int main(int argc, char** argv){
	
	int option = 0;
	while((option = getopt(argc, argv, "ldfs"))!=-1){
		switch(option){

			case 'l':
				arrayofFlags[0] = 1;
				break;
			case 'd':
				arrayofFlags[1] = 1;
				break;
			case 'f':
				arrayofFlags[2] = 1;
				break;
			case 's':
				arrayofFlags[3] = 1;
				break;
			case '?':
				break;
		}
	}	

	int numOfNonOptions=0;
	int dirIndex=0;
	for (int i=1;i<argc;i++){
		if (argv[i][0]!='-'){
			numOfNonOptions++;
			dirIndex = i;
		}		
	}
	if (numOfNonOptions!=1 && numOfNonOptions!=0){
		printf("Can`t find directory path in params as you are input more then 1 possible way! Try again. \n");
		exit(99);
	}
	if (numOfNonOptions){
		dirwalk(argv[dirIndex]);
	}
	else if (numOfNonOptions==0){
		char* b = (char*)malloc(2);
		b[0] = '.';
		b[1] = '\0';
		dirwalk(b);
		free(b);
	}
    
}	