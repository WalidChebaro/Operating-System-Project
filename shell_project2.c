#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define SIZE_OF_COMMAND_LINE 100

/* Selected Commands of the Shell */
void cmd_exit();
void cmd_help();
int line_reader();
void executes();
void main_loop();

char cmd[SIZE_OF_COMMAND_LINE];

typedef struct cmd_doc
{
    char *cmd;
    char *desc;
} cmd_doc;

cmd_doc cmd_table[] =
    {{"cd", "cd - change the directory"},
     {"help", "help - show the help menu"},
     {"exit", "exit - exit the shell"},
     {NULL, "Use man command to get help for the other commands"}};

void cmd_help()
{

    int i;
    for (i = 0; i < 10; i++)
    {
        printf("%s\n", cmd_table[i].desc);
    }
}

void cmd_exit()
{
    exit(0);
}

void main_loop()
{
    int i;

    while (1)
    {
        printf("%d: ", i);
        if (line_reader() == 1)
            ;
        else
        {
            executes();
        }
        i++;
    }
}

int line_reader()
{
    char input[SIZE_OF_COMMAND_LINE];
    int index = 0;
    int i;
    while (scanf("%c", &input[index]))
    {
        if (input[index] == '\n')
        {
            break;
        }
        index++;
    }

    if (index == 0)
    {
        return 1;
    }
    else
    {
        for (i = 0; i <= sizeof(input); i++)
        {
            cmd[i] = input[i];
        }
        return 0;
    }
}

void executes()
{
    int i;
    char *token[10];
    token[0] = strtok(cmd, " \n");
    while (token[i] != NULL)
    {
        i++;
        token[i] = strtok(NULL, " \n");
    }

    if (strcmp(token[0], cmd_table[0].cmd) == 0 || strcmp(token[0], cmd_table[1].cmd) == 0 || strcmp(token[0], cmd_table[2].cmd) == 0)
    {
        if (strcmp(token[0], cmd_table[0].cmd) == 0)
        {
            chdir(token[1]);
        }

        else if (strcmp(token[0], cmd_table[1].cmd) == 0)
        {
            cmd_help();
        }

        else if (strcmp(token[0], cmd_table[2].cmd) == 0)
        {
            cmd_exit();
        }
    }
    else
    {
        pid_t pid = fork();

        if (pid == 0)
        {
            execvp(token[0], token);
        }

        if (pid > 0)
        {
            wait(NULL);
        }
        else
        {
            perror("Error forking child.");
        }
    }
}

int main(int argc, char *argv[])
{
    main_loop();
    return 0;
}
