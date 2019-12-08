/*  ----- Remote CLI Shell Server -----  */

//      Walid Chebaro
//      Joseph Zakher
//      Christophe Jabbour

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERV_PORT 8080
#define BACKLOG 3
#define SIZE_OF_LINE 1024
#define NUMBER_OF_CONNECTION 100

/* Initializing commands of the shell */
void cmd_help(int);
void exec(int, char[], char *[]);
void exec_pipes(int, char[], char *[], char *[]);
void main_loop(int);
void token_space(char[], char *[]);

int token_pipe(char[], char *[]);
void *server_handler(void *);

/* Implementing the help function using a struct cmd_doc of command 
and their description, inserting them in an array cmd_table[],
and printing them using the method cmd_help() */
typedef struct cmd_doc
{
    char *cmd;
    char *desc;
} cmd_doc;

cmd_doc cmd_table[] =
    {{"cd", "cd - change the directory\n"},
     {"help", "help - show the help menu\n"},
     {"stop", "stop - stops the server\n"},
     {NULL, "Use man command to get help for the other commands\n"}};

void cmd_help(int client_socket)
{
    char help[strlen(cmd_table[0].desc) + strlen(cmd_table[1].desc) + strlen(cmd_table[2].desc) + strlen(cmd_table[3].desc) + 1];

    strcpy(help, cmd_table[0].desc);
    strcat(help, cmd_table[1].desc);
    strcat(help, cmd_table[2].desc);
    strcat(help, cmd_table[3].desc);

    send(client_socket, help, sizeof(help), 0);
}

/* In the main loop, we're printing each line number on the shell,
and getting the user input using the cmd array. If the user entered a "|", 
we're executing the command using pipes, and normally otherwise. */
void main_loop(int client_socket)
{
    while (1)
    {
        char cmd[SIZE_OF_LINE];
        char cwd[SIZE_OF_LINE];
        getcwd(cwd, sizeof(cwd));
        send(client_socket, getcwd(cwd, sizeof(cwd)), sizeof(cwd), 0);
        read(client_socket, cmd, sizeof(cmd));
        int j;
        int p = 0;
        for (j = 0; j < sizeof(cmd); j++)
        {
            if (cmd[j] == '|')
            {
                p = 1; // Used to set shell to pipe mode
            }
        }
        if (p == 0)
        {
            char *token[10];
            exec(client_socket, cmd, token); // Normal execution
        }
        else
        {
            char *token[10];
            char *token_p[10];
            exec_pipes(client_socket, cmd, token, token_p); // Execution using pipes
        }
    }
}

/* If the shell is in normal mode (no piping needed),
we're splitting the user input into tokens using " " as delimiter */
void token_space(char line[], char *token[])
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
int token_pipe(char line[], char *token_p[])
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
void exec(int client_socket, char cmd[], char *token[])
{
    token_space(cmd, token);

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
                cmd_help(client_socket);
            }

            else
            {
                dup2(client_socket, 1);
                dup2(client_socket, 2);
                close(client_socket);
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

void exec_pipes(int client_socket, char cmd[], char *token[], char *token_p[])
{
    // Tokenizing on pipes and ditermening the number of pipes
    int pipe_number = token_pipe(cmd, token_p);
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
    {
        pipe(pipes + j);
    }

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
                dup2(pipes[totalPipes - 1], 1);
                // Closing all the other pipes ends as they are not needed
                for (int j = 0; j < totalPipes; j++)
                {
                    close(pipes[j]);
                }

                token_space(token_p[i], token);

                if (execvp(token[0], token) < 0)
                {
                    printf("Could not execute command at pipe 1\n");
                    exit(0);
                }
            }
            else if (i == command_number - 1)
            {
                // Dup is used to receive input from the pipe read end (0)
                dup2(pipes[totalPipes - 2], 0);

                // Closing the unused pipe ends
                for (int j = 0; j < totalPipes; j++)
                {
                    close(pipes[j]);
                }

                token_space(token_p[i], token);
                dup2(client_socket, 1);
                dup2(client_socket, 2);
                close(client_socket);
                if (execvp(token[0], token) < 0)
                {
                    printf("Could not execute command at pipe %d\n", i + 1);
                    exit(0);
                }
            }
            else
            {
                // Pipe read end will go to the stdin process
                dup2(pipes[i + (i - 2)], 0);
                // Process stdout will go to the pipe write end
                dup2(pipes[i + i + 1], 1);
                // Closing the unused pipe ends
                for (int j = 0; j < totalPipes; j++)
                {
                    close(pipes[j]);
                }

                token_space(token_p[i], token);

                if (execvp(token[0], token) < 0)
                {
                    printf("Could not execute command at pipe %d\n", i);
                    exit(0);
                }
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
    {
        close(pipes[j]);
    }

    // Wait until all processes are finished
    for (int i = 0; i < command_number; i++)
    {
        waitpid(pid[i], NULL, 0);
    }
}

void *server_handler(void *arg)
{
    int client_socket = *(int *)arg;
    main_loop(client_socket);
    close(client_socket);
    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    time_t ticks;
    int opt = 1;
    int server_socket;                       // server socket used to listen and accept incoming connections
    int client_socket[NUMBER_OF_CONNECTION]; // client socket used to communicate with a connected client
    int client_counter = 0;
    struct sockaddr_in servaddr, client; // a struct that holds information about internet address
    pthread_t thread[NUMBER_OF_CONNECTION];
    pthread_attr_t attr; // attribute object for the thread
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); // make the thread detachable

    /* Create an unbound socket and return a file descriptor
       int socket(int family, int type, int protocol)
    */
    // IPV4, STREAM, TCP
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(1);
    }
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt Failed");
        exit(EXIT_FAILURE);
    }

    servaddr.sin_family = AF_INET;                // set the domain
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // set the IP without knowing the IP of the running system
    servaddr.sin_port = htons(SERV_PORT);         // set the port number

    /* Assign a local socket address (servaddr) to a socket identified by a descriptor (server_socket)
       bind(int sockfd, struct sockaddr *my_addr, int addrlen)
    */
    if (bind(server_socket, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind failed");
        exit(1);
    }

    if (listen(server_socket, BACKLOG) < 0)
    {
        perror("listen failed");
        exit(1);
    }

    while (1)
    {
        client_socket[client_counter] = accept(server_socket, (struct sockaddr *)NULL, NULL);
        if (client_socket[client_counter] < 0)
            continue;
        if (pthread_create(&thread[client_counter], &attr, server_handler, &client_socket[client_counter]) < 0)
        {
            perror("could not create thread");
            return 1;
        }
        client_counter++; // increment counter
        /* Print current time and number of served clients */
        ticks = time(NULL);
        printf("Client %d connected at %s\n", client_counter, ctime(&ticks));
    }
    close(server_socket);
    return 0;
}