
/* CLient side */
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SIZE_OF_LINE 1024
#define SERV_PORT 8080
#define SIZE_OF_COMMAND_LINE 100

int main(int argc, char *argv[])
{
    int client_socket = 0; // client socket used to communicate with a server
    struct sockaddr_in servaddr;
    /* Create a socket with the specified domain and protocol */
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error");
        exit(1);
    }
    servaddr.sin_family = AF_INET;        // set the domain
    servaddr.sin_port = htons(SERV_PORT); // set the port number

    /* Convert (into a network address) and set the IP */
    if (inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr) <= 0)
    {
        printf("inet_pton error for 127.0.0.1");
        exit(1);
    }
    /* Attempt to make a connection on the socket */
    if (connect(client_socket, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("connect error");
        exit(1);
    }

    while (1)
    {
        char line[SIZE_OF_LINE];
        char output[SIZE_OF_LINE];
        char cmd[SIZE_OF_COMMAND_LINE];
        int index = 0;
        int esc = 0;
        int no_output = 2;
        read(client_socket, line, SIZE_OF_LINE);
        while (no_output == 2)
        {
            printf("WJC_SHELL%s: ", line);

            fgets(cmd, sizeof(cmd), stdin); //read from stdinof user the command
            if (cmd[0] == '\n')
            {
                no_output = 2;
            }
            else if (cmd[0] == 'c' && cmd[1] == 'd')
            {
                no_output = 1;
            } //if command is cd
            else if (cmd[0] == '.' && cmd[1] == '/')
            {
                no_output = 1;
            } //if command starts with ./
            else if (cmd[0] == 'r' && cmd[1] == 'm')
            {
                no_output = 1;
            } //if command is rm
            else if (cmd[0] == 'm' && cmd[1] == 'k' && cmd[2] == 'd' && cmd[3] == 'i' && cmd[4] == 'r')
            {
                no_output = 1;
            } //if command is mkdir
            else if (cmd[0] == 'e' && cmd[1] == 'x' && cmd[2] == 'i' && cmd[3] == 't')
            {
                no_output = 1;
                esc = 1;
            } //if command is help
            else
            {
                no_output = 0;
            }
        }

        memset(line, 0, sizeof(line));
        send(client_socket, cmd, sizeof(cmd), 0);
        if (no_output == 0)
        {
            read(client_socket, output, SIZE_OF_LINE);
            printf("\n%s\n", output);
        }
        if (esc == 1)
        {
            exit(1);
        }
        memset(output, 0, sizeof(output));
    }

    return 0;
}
