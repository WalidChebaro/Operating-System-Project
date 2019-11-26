/*  ----- Remote CLI Shell Server -----  */

//      Walid Chebaro
//      Joseph Zakher
//      Christophe Jabbour

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define SERV_PORT 8080
#define BACKLOG 3
#define SIZE_OF_COMMAND_LINE 100
#define SIZE_OF_CWD 1024

/* Initializing commands of the shell */
void cmd_help();
void exec();
void exec_pipes();
void main_loop(int);
void token_space(char[]);

int line_reader(int);
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
void main_loop(int client_socket)
{
    int i = 0;
    char cwd[SIZE_OF_CWD];
    while (1)
    {
        getcwd(cwd, sizeof(cwd));
        send(client_socket, getcwd(cwd, sizeof(cwd)), SIZE_OF_CWD, 0);
        //printf("WJC_SHELL%s: ", cwd);

        // If empty command line, do nothing
        if (line_reader(client_socket) == 1)
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
                exec(client_socket);
            }
            else
            {
                exec_pipes(client_socket); // Execution using pipes
            }
            memset(token, 0, sizeof(token));
            memset(cmd, 0, sizeof(cmd));
        }
        //  i++; // Increment line number
    }
}

/* The user's input is scanned and stored in the cmd array, 
which is a global variable, and which will be used in the main loop */
int line_reader(int client_socket)
{

    int index = 0;
    char cmd_from_client[SIZE_OF_COMMAND_LINE];
    read(client_socket, cmd_from_client, SIZE_OF_COMMAND_LINE);

    while (cmd[index] = cmd_from_client[index])
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
void exec(int client_socket)
{
    token_space(cmd);

    // Comparing user input (command) to our custom table of commands
    if (strcmp(token[0], cmd_table[2].cmd) == 0)
    {dup2(client_socket, 1);
        // Terminate shell
        exit(0);
    }
    else if (strcmp(token[0], cmd_table[0].cmd) == 0)
    {dup2(client_socket, 1);
        // Change directory
        chdir(token[1]);
    }
    else
    {
        pid_t pid = fork();

        if (pid == 0)
        {
            if (strcmp(token[0], cmd_table[1].cmd) == 0)
            {dup2(client_socket, 1);
                // HELP, Provides info to the user
                cmd_help();
            }

            else
            {
                dup2(client_socket, 1);
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

void exec_pipes(int client_socket)
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

    int server_socket;           // server socket used to listen and accept incoming connections
    int client_socket;           // client socket used to communicate with a connected client
    struct sockaddr_in servaddr; // a struct that holds information about internet address

    /* Create an unbound socket and return a file descriptor
       int socket(int family, int type, int protocol)
    */
    // IPV4, STREAM, TCP

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(1);
    }

    servaddr.sin_family = AF_INET;         // set the domain
    servaddr.sin_addr.s_addr = INADDR_ANY; // set the IP without knowing the IP of the running system
    servaddr.sin_port = htons(SERV_PORT);  // set the port number

    /* Assign a local socket address (servaddr) to a socket identified by a descriptor (server_socket)
       bind(int sockfd, struct sockaddr *my_addr, int addrlen)
    */
    if (bind(server_socket, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind failed");
        exit(1);
    }
    /* Mark the socket to be used to accept connection requests
       Set the maximum size of the pending connection queue
    */
    if (listen(server_socket, BACKLOG) < 0)
    {
        perror("listen failed");
        exit(1);
    }

    if ((client_socket = accept(server_socket, (struct sockaddr *)NULL, NULL)) < 0)
    {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
    main_loop(client_socket);
    // We first fork to start the shell as a new process over the kernel
    // pid_t p = fork();

    // if (p == 0)
    // {
    //     main_loop(client_socket);
    // }

    // else if (p > 0)
    // {
    //     wait(NULL);
    // }

    // else
    // {
    //     printf("failed to fork\n");
    //     exit(1);
    // }

    return 0;
}
