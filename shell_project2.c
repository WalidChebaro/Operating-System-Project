/*  ----- Local CLI Shell -----  */

//      Walid Chebaro
//      Christophe Jabbour
//      Joseph Zakher

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define SIZE_OF_COMMAND_LINE 100

/* Initializing commands of the shell */
void cmd_help();
int line_reader();
void exec();
void exec_pipes();
void main_loop();

void token_space(char[]);
void token_pipe(char[]);
char *token[10];
char *token_p[10];
char cmd[SIZE_OF_COMMAND_LINE];

/* Implementing the help function using a struct cmd_doc of command 
and their description, inserting them in an array cmd_table[],
and printing them using the method cmd_help() */
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

/* In the main loop, we're printing each line number on the shell,
and getting the user input using the cmd array. If the user entered a "|", 
we're executing the command using pipes, and normally otherwise. */
void main_loop()
{
    int i = 0;

    while (1)
    {
        printf("%d: ", i);

        // If empty command line, do nothing
        if (line_reader() == 1)
        {
            ;
        }

        else
        {
            int j;
            int p = 0;
            for (j = 0; j < sizeof(cmd); j++)
            {
                if (cmd[j] == '|')
                {
                    p = 1; // Used to set shell to pipe mode
                }
            }
            if (p == 0) // Normal execution
            {
                exec();
            }
            else
            {
                exec_pipes(); // Execution using pipes
            }
        }
        i++; // Increment line number
    }
}

/* The user's input is scanned and stored in the cmd array, 
which is a global variable, and which will be used in the main loop */
int line_reader()
{
    char input[SIZE_OF_COMMAND_LINE];
    int index = 0;
    int i;

    while (scanf("%c", &input[index]))
    {
        // Check for empty command line
        if (input[index] == '\n')
        {
            break;
        }
        index++;
    }

    // If empty, return 1, used in main_loop to do nothing until user inputs a command.
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

/* If the shell is in normal mode (no piping needed),
we're splitting the user input into tokens using " " as delimiter */
void token_space(char line[])
{
    char *tok;
    int i = 0;

    tok = strtok(line, " \n");
    while (tok != NULL)
    {
        token[i] = tok;
        i++;
        tok = strtok(NULL, " \n");
    }
    token[i] = NULL;
}

/* In piping mode, we're delimiting the user input by searching for "|"
we then tokenize again using " " as delimiter to get each command separately */
void token_pipe(char line[])
{
    char *tok;
    int i = 0;

    tok = strtok(line, "|");
    while (tok != NULL)
    {
        token_p[i] = tok;
        i++;
        tok = strtok(NULL, "|");
    }
    token_p[i] = NULL;
}

/* To execute the commands entered by the user, we start by calling token_space()
to get each command on a single line separately. We check first for the presence
of our custom functions and execute them, then we fork to use execvp and run normal 
shell commands. If we don't create a child process to run a new program (execvp), 
the function will execute then the shell will terminate automatically */
void exec()
{
    token_space(cmd);
    // Comparing user input (command) to our custom table of commands
    if (strcmp(token[0], cmd_table[2].cmd) == 0)
    {
        // Terminate shell
        exit(0);
    }
    else if (strcmp(token[0], cmd_table[0].cmd) == 0)
    {
        // Change directory
        chdir(token[1]);
    }
    else
    {
        pid_t pid = fork();

        if (pid == 0)
        {
            if (strcmp(token[0], cmd_table[1].cmd) == 0)
            {
                // HELP, Provides info to the user
                cmd_help();
            }

            else
            {
                // Run any other (usual) shell command
                execvp(token[0], token);
            }
            exit(0);
        }

        else if (pid > 0)
        {
            // Parent wait for child to finish execution
            wait(NULL);
        }

        else
        {
            perror("fork failed");
            exit(1);
        }
    }
    memset(token, 0, sizeof(token));
}

/* In pipe mode, first we start by tokenizing the user input, */
void exec_pipes()
{
    token_pipe(cmd);

    int pipefd[2];
    pid_t pid;

    pipe(pipefd);

    pid = fork();

    if (pid == 0)
    {
        close(pipefd[1]);
        dup2(pipefd[0], 0);
        token_space(token_p[1]);

        if (execvp(token[0], token) < 0)
        {
            printf("Could not execute command before pipe.");
            exit(0);
        }

        memset(token, 0, sizeof(token));
    }

    else if (pid>0)
    {
        close(pipefd[0]);
        dup2(pipefd[1], 1);

        token_space(token_p[0]);

        if (execvp(token[0], token) < 0)
        {
            printf("Could not execute command after pipe.");
            exit(0);
        }

        memset(token, 0, sizeof(token));
    }

    else
    {
        printf("failed to fork");
        return;
    }
    memset(token_p, 0, sizeof(token_p));
}

int main(int argc, char *argv[])
{
    pid_t p = fork();

    if (p == 0)
    {
        main_loop();
    }

    else if (p > 0)
    {
        wait(NULL);
    }

    else
    {
        printf("failed to fork");
    }

    return 0;
}
