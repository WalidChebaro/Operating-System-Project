/*  ----- Local CLI Shell -----  */

//      Walid Chebaro
//      Joseph Zakher
//      Christophe Jabbour

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define SIZE_OF_COMMAND_LINE 100

/* Initializing commands of the shell */
void cmd_help();
void exec();
void exec_pipes();
void main_loop();
void token_space(char[]);

int line_reader();
int token_pipe(char[]);

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
    for (i = 0; i < 4; i++)
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
    char cwd[1024];
    while (1)
    {
        getcwd(cwd, sizeof(cwd));
        printf("WJC_SHELL%s: ", cwd);
        // printf("%d: ", i);

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
            memset(token, 0, sizeof(token));
            memset(cmd, 0, sizeof(cmd));
        }
        //  i++; // Increment line number
    }
}

/* The user's input is scanned and stored in the cmd array, 
which is a global variable, and which will be used in the main loop */
int line_reader()
{

    int index = 0;

    while (scanf("%c", &cmd[index]))
    {
        // Check for empty command line
        if (cmd[index] == '\n')
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
        return 0;
    }
}

/* If the shell is in normal mode (no piping needed),
we're splitting the user input into tokens using " " as delimiter */
void token_space(char line[])
{
    char *tok;
    int i = 0;

    tok = strtok(line, " \t\r\n\a");
    while (tok != NULL)
    {
        token[i] = tok;
        i++;
        tok = strtok(NULL, " \t\r\n\a");
    }
    token[i] = NULL;
}

/* In piping mode, we're delimiting the user input by searching for "|"
we then tokenize again using " " as delimiter to get each command separately */
int token_pipe(char line[])
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
    return i - 1;
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
                if (execvp(token[0], token) < 0)
                {
                    printf("Could not execute command\n");
                    exit(0);
                }
            }
        }

        else if (pid > 0)
        {
            // Parent wait for child to finish execution
            wait(NULL);
        }

        else
        {
            printf("failed to fork");
            exit(1);
        }
    }
}

void exec_pipes()
{
    // Tokenizing on pipes and ditermening the number of pipes
    int pipe_number = token_pipe(cmd);
    int command_number = pipe_number + 1;
    // The number of processes are equal to the number of commands
    int pid[command_number];
    // The total number of pipes to be made is twice the number of pipes that the user inserted
    // in order to write and read.
    int totalPipes = (pipe_number)*2;
    int pipes[totalPipes];

    // Initializing pipes
    int j;
    for (j = 0; j < totalPipes; j += 2)
        pipe(pipes + j);

    // We create processes depending on the number of commands
    // Each process executes a command
    int i;
    for (i = 0; i < command_number; i++)
    {
        // For each command we create a new process
        pid[i] = fork();

        if (pid[i] == 0)
        {

            if (i == 0)
            {

                // Dup is used to send the output to the pipe write end (1)
                close(1);
                dup(pipes[1]);

                // Closing all the other pipes ends as they are not needed
                for (int j = 0; j < totalPipes; j++)
                    close(pipes[j]);

                token_space(token_p[i]);

                if (execvp(token[0], token) < 0)
                {
                    printf("Could not execute command at pipe %d\n", i);
                    exit(0);
                }

                memset(token, 0, sizeof(token)); // Resetting the array of tokens
            }
            else if (i == command_number - 1)
            {
                // Dup is used to receive input from the pipe read end (0)
                close(0);
                dup(pipes[totalPipes - 2]);

                // Closing the unused pipe ends
                for (int j = 0; j < totalPipes; j++)
                    close(pipes[j]);

                token_space(token_p[i]);

                if (execvp(token[0], token) < 0)
                {
                    printf("Could not execute command at pipe %d\n", i + 2);
                    exit(0);
                }

                memset(token, 0, sizeof(token)); // Resetting the array of tokens
            }
            else
            {
                // Pipe read end will go to the stdin process
                close(0);
                dup(pipes[i + (i - 2)]);
                // Process stdout will go to the pipe write end
                close(1);
                dup(pipes[i + i + 1]);

                // Closing the unused pipe ends
                for (int j = 0; j < totalPipes; j++)
                    close(pipes[j]);

                token_space(token_p[i]);

                if (execvp(token[0], token) < 0)
                {
                    printf("Could not execute command at pipe %d\n", i + 1);
                    exit(0);
                }

                memset(token, 0, sizeof(token)); // Resetting the array of tokens
            }
        }
        else if (pid[i] < 0)
        {
            printf("failed to fork\n");
            exit(1);
        }
    }

    // Closing the unused pipe ends
    for (int j = 0; j < totalPipes; j++)
        close(pipes[j]);

    // Wait until all processes are finished
    for (int i = 0; i < command_number; i++)
        waitpid(pid[i], NULL, 0);
}

int main(int argc, char *argv[])
{
    // We first fork to start the shell as a new process over the kernel
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
        printf("failed to fork\n");
        exit(1);
    }

    return 0;
}
