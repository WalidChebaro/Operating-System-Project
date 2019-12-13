/*  ----- Remote CLI Shell Server -----  */

//      Walid Chebaro
//      Joseph Zakher
//      Christophe Jabbour

#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SIZE_OF_LINE 1024
#define SERV_PORT 8080
#define SIZE_OF_COMMAND_LINE 100

int main(int argc, char *argv[])
{

    int client_socket; // client socket used to communicate with a server
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
        int stop = 0;

        if (read(client_socket, line, sizeof(line)) >= 0)
        {
            printf("WJC_SHELL%s: ", line);
        }

        memset(line, 0, SIZE_OF_LINE);

        fgets(line, sizeof(line), stdin); //read from stdinof user the command

        if (line[0] == 'e' && line[1] == 'x' && line[2] == 'i' && line[3] == 't')
        {
            exit(1);
        } //if command is stop
        if (line[0] == 's' && line[1] == 't' && line[2] == 'o' && line[3] == 'p')
        {
            stop = 1;
        } //if command is stop

        send(client_socket, line, sizeof(line), 0);

        memset(line, 0, SIZE_OF_LINE);

        if (read(client_socket, line, sizeof(line)) >= 0)
        {
            printf("%s", line);
        }
        
        if (stop == 1)
        {
            exit(1);
        }
    }
    close(client_socket);

    return 0;
}