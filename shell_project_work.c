#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define SIZE_OF_COMMAND_LINE 100
#define NUMBER_OF_COMMAND_LINE 1024

/* Selected Commands of the Shell */
void cmd_exit();
void cmd_help();
void cmd_pwd();
void cmd_cd();
void cmd_ls();
void cmd_mkdir();
void cmd_create_file();
void cmd_delete_file();
void command_definer(int);

typedef struct cmd_line
{
    char cmd_line_content[SIZE_OF_COMMAND_LINE];
} cmd_line;

cmd_line cmd[NUMBER_OF_COMMAND_LINE];

typedef struct cmd_doc
{
    char *cmd;
    char *desc;
} cmd_doc;

cmd_doc cmd_table[] =
    {{"help", "help - show the help menu"},
     {"exit", "exit - exit the shell"},
     {"pwd", "pwd - print the working directory"},
     {"cd", "cd - change the directory"},
     {"ls", "ls - list the current directory"},
     {"mkdir", "mkdir - create a directory"},
     {"create_file", "create_file - create a new file"},
     {"delete_file", "delete_file - delete the current file"}};

void cmd_help(int i)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        int i;
        for (i = 0; i < 9; i++)
        {
            printf("%s\n", cmd_table[i].desc);
        }
        if (pid > 0)
        {
            wait(NULL);
            command_definer(i);
        }

        if (pid < 0)
        {
            perror("ERROR");
        }
    }
}

void cmd_exit()
{
    exit(0);
}

void cmd_pwd(int i)
{
    pid_t pid = fork();

    if (pid == 0)
    {
        execlp("pwd", "pwd", NULL);

        if (pid > 0)
        {
            wait(NULL);
            command_definer(i);
        }

        if (pid < 0)
        {
            perror("ERROR");
        }
    }
}

void cmd_cd()
{
}

void cmd_ls(int i)
{
    pid_t pid = fork();

    if (pid == 0)
    {
        execlp("ls", "ls", "-l", NULL);
        if (pid > 0)
        {
            wait(NULL);
            command_definer(i);
        }

        if (pid < 0)
        {
            perror("ERROR");
        }
    }

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

// Divides the line from the command array into token and assignes the corresponding cmd.
void command_definer(int i)
{
    char *token = strtok(cmd[i + 1].cmd_line_content, " ");

    if (strcmp(token, cmd_table[0].cmd) == 0)
    {
        cmd_help(i);
    }

    if (strcmp(token, cmd_table[1].cmd) == 0)
    {
        cmd_exit();
    }

    if (strcmp(token, cmd_table[2].cmd) == 0)
    {
        cmd_pwd(i);
    }

    if (strcmp(token, cmd_table[3].cmd) == 0)
    {
        cmd_cd();
    }

    if (strcmp(token, cmd_table[4].cmd) == 0)
    {
        cmd_ls(i);
    }

    if (strcmp(token, cmd_table[5].cmd) == 0)
    {
        cmd_mkdir();
    }

    if (strcmp(token, cmd_table[6].cmd) == 0)

    {
        cmd_create_file();
    }

    if (strcmp(token, cmd_table[7].cmd) == 0)
    {
        cmd_delete_file();
    }
}

// Scans the line from the user and fills the command array with the line.
void line_reader(int i)
{
    scanf("%s", &cmd[i].cmd_line_content[SIZE_OF_COMMAND_LINE]);
}

void main_loop()
{
    char cmd;
    int i;

    for (i = 0; i < NUMBER_OF_COMMAND_LINE; i++)
    {
        printf("%d: ", i);
        line_reader(i);
        command_definer(i);
    }
}

int main(int argc, char *argv[])
{
    /* start a program over the one running at all time, the kernel.
    But it is a copy of the kernel, that's why we use exec to run a 
    different program*/
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("ERROR");
    }

    if (pid == 0)
    {
        main_loop();
    }

    if (pid > 0)
    {
        wait(NULL);
    }
}