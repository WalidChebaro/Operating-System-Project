
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
        printf("inet_pton error for %s", argv[1]);
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
        int index = 0;
        read(client_socket, line, SIZE_OF_LINE);

        printf("WJC_SHELL%s: ", line);

        fgets(line, sizeof(line), stdin);

        send(client_socket, line, sizeof(line), 0);
        read(client_socket, line, SIZE_OF_LINE);
        printf("%s\n", line);
    }
    return 0;
}
