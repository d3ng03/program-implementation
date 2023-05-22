#include <stdio.h>
#include <string.h>
 
#define BUFF_SIZE 1024

void check_canary();
void check_dep();
void check_pie();

int main(int argv, char* argc[]){

    if(argv < 2){
        printf("Usage: mychecksec [ELF FILE NAME]");
        exit(0);
    }

    for(int i=1;i < argv;i++){
        FILE *fp = fopen(argc[i], "r");

        if (fp == NULL) {
            printf("File '%s' does not exist.\n", argc[i]);
            exit(0);
        }
        fclose(fp);

        printf("File Name: %s\n",argc[i]);
        check_canary(argc[i]);
        check_dep(argc[i]);
        check_pie(argc[i]);
    }

    return 0;
}

void check_canary(char* file){
    char buf[BUFF_SIZE] = {0,};
    char command[BUFF_SIZE] = {0,};
    sprintf(command,"objdump -t %s | grep '__stack_chk'",file);
    FILE *fp;
    //printf("command: %s\n",command);
    fp = popen(command, "r");
    if (fp == NULL){
        printf("failed to execute command, try again\n");
        exit(0);
    }
 
    fgets(buf, BUFF_SIZE, fp);
    //printf("buf: %d\n",strstr(buf,"__stack_chk"));
    
    if(strstr(buf,"__stack_chk") == 0){
        printf("%-15s\033[1;31m Canary disabled \033[0m \n","[*]Canary:");
    }else{
        printf("%-15s\033[1;32m Canary found \033[0m \n","[*]Canary:");
    }
 
    pclose(fp);
 
    return 0;
}

void check_dep(char* file){
    char buf[BUFF_SIZE] = {0,};
    char command[BUFF_SIZE] = {0,};
    sprintf(command,"readelf -l %s | grep -A1 'GNU_STACK'",file);
    FILE *fp;
    //printf("command: %s\n",command);
    fp = popen(command, "r");
    if (fp == NULL){
        printf("failed to execute command, try again\n");
        exit(0);
    }
 
    fgets(buf, BUFF_SIZE, fp);
    //printf("buf: %d\n",strstr(buf,"RWE"));
    
    if(strstr(buf,"RWE") == 0){
        printf("%-15s\033[1;32m DEP enabled \033[0m \n","[*]DEP:");
    }else{
        printf("%-15s\033[1;31m DEP disabled \033[0m \n","[*]DEP:");
    }
 
    pclose(fp);
 
    return 0;
}

void check_pie(char* file){
    char buf[BUFF_SIZE] = {0,};
    char command[BUFF_SIZE] = {0,};
    sprintf(command,"readelf -h %s | grep 'Type:'",file);
    FILE *fp;
    //printf("command: %s\n",command);
    fp = popen(command, "r");
    if (fp == NULL){
        printf("failed to execute command, try again\n");
        exit(0);
    }
 
    fgets(buf, BUFF_SIZE, fp);
    //printf("buf: %s\nresult: %d\n",buf,strstr(buf,"DYN"));
    
    if(strstr(buf,"DYN") == 0){
        printf("%-15s\033[1;31m PIE disabled \033[0m \n","[*]PIE:");
    }else{
        printf("%-15s\033[1;32m PIE enabled \033[0m \n","[*]PIE:");
    }
 
    pclose(fp);
    printf("\n");
    return 0;
}