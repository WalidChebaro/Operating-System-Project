#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/* Selected Commands of the Shell */
void cmd_exit();
void cmd_help();
void cmd_pwd();
void cmd_cd();
void cmd_ls();
void cmd_mkdir();
void cmd_chdir();
void cmd_create_file();
void cmd_delete_file();





char cmd_table[]= 
    {"help - show the help menu",
    "exit - exit the shell",
    "pwd - print the working directory",
    "cd - change the directory",
    "ls - list the current directory",
    "mkdir - create a directory",
    "chdir - change the current directory",
    "create_file - create a new file",
    "delete_file - delete the current file"};

void cmd_help()
{
    int i;
    for (i=0;i<sizeof(cmd_table);i++)
    {
        printf("%c\n", cmd_table[i]);    
    }
}

void cmd_exit()
{
    exit(0);
}

void cmd_pwd()
{  
    execlp("pwd", "pwd", NULL);
}

void cmd_cd()
{

}

void cmd_ls()
{
    execlp("ls", "ls", "-l", NULL);
}

void cmd_mkdir()
{

}

void cmd_chdir()
{

}

void cmd_create_file()
{

}

void cmd_delete_file()
{

}   

void main_loop(){
    char line;
    char command[1024];
    int i, j;
        do{
            for(i=0; i<sizeof(command);i++)
            {
            printf("%d: ", i);    
            scanf("%s", &command[i]);

            for(j=0;j<30;j++)
            {
                printf("%c", command[j]);
            }

            }

            
        }while(1);
}

int main(int argc, char *argv[])
{
    /* start a program over the one running at all time, the kernel.
    But it is a copy of the kernel, that's why we use exec to run a 
    different program*/
    pid_t pid = fork();
    
    
    while(1)
    {
    if(pid<0)
    {
        perror("ERROR");
    }

    if(pid==0)
    {
        main_loop();
    }

    if(pid>0)
    {
        wait(NULL);
    }
    }
}