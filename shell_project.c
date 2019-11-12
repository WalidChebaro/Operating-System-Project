#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

/* Selected Commands of the Shell */
void cmd_exit();
void cmd_help();
void cmd_pwd();
void cmd_cd();
void cmd_ls();
void cmd_mkdir();
void cmd_create_file();
void cmd_delete_file();
char command[][50]={'\0'};

//void cmd_test(){
//     int i;
//     for(i=0;i<3;i++)
//     {
//         printf("%s\n", command[i]);
//     }
// }

char helper_cmd_table[] = {
    {"?"},
    {"exit"},
    {"pwd"},
    {"cd"},
    {"ls"},
    {"mkdir"},
    {"create_file"},
    {"delete_file"}};

char cmd_table[8][50]= 
    {"help - show the help menu",
    "exit - exit the shell",
    "pwd - print the working directory",
    "cd - change the directory",
    "ls - list the current directory",
    "mkdir - create a directory",
    "create_file - create a new file",
    "delete_file - delete the current file"};

void cmd_help()
{
    int i;
    for (i=0;i<9;i++)
    {
        printf("%s\n", cmd_table[i]);    
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
    char cmd;
    int i;

    for(i = 0; i < sizeof(command); i++)
    {
        printf("%d: ", i);    
        scanf("%s", &line);

        if(line == '\0' || line == NULL)
        {
            printf("%d: ", i++);
        }else{
            command[i]=line;
        }

        // duplicate string to use its substring
        strcpy(cmd, line);

        if (strstr(cmd, helper_cmd_table[0]))
        {
            cmd_help();
        }

        if (strstr(cmd, helper_cmd_table[1]))
        {
            cmd_exit();
        }

        if (strstr(cmd, helper_cmd_table[2]))
        {
            cmd_pwd();
        }

        if (strstr(cmd, helper_cmd_table[3]))
        {
            cmd_cd();
        }

        if (strstr(cmd, helper_cmd_table[4]))
        {
            cmd_ls();
        }

        if (strstr(cmd, helper_cmd_table[5]))
        {
            ccmd_mkdir();
        }

        if (strstr(cmd, helper_cmd_table[6]))
        {
            cmd_create_file();
        }

        if (strstr(cmd, helper_cmd_table[7]))
        {
            cmd_delete_file();
        }
    }

   // cmd_test();

}


 
int main(int argc, char *argv[])
{
    /* start a program over the one running at all time, the kernel.
    But it is a copy of the kernel, that's why we use exec to run a 
    different program*/
    pid_t pid = fork();
    
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