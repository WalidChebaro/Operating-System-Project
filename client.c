
/* CLient side */
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define MAXLINE 100

void *send_message(void *);

int main(int argc, char *argv[])
{
    void *status = 0;
    int client_socket; // client socket used to communicate with a server
    struct sockaddr_in servaddr;

    /* Ensure than at least one argument is present (IP address) */
    if (argc != 3)
    {
        perror("usage: please specify IP address and port number to connect to");
        exit(1);
    }
    /* Create a socket with the specified domain and protocol */
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error");
        exit(1);
    }

    servaddr.sin_family = AF_INET;            // set the domain
    servaddr.sin_port = htons(atoi(argv[2])); // set the port number

    /* Convert (into a network address) and set the IP */
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
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
    char buff[MAXLINE];
    scanf("%s", buff);
    write(client_socket, buff, strlen(buff));

    char echo[MAXLINE];
    bzero(echo, MAXLINE);
    int  n = read(client_socket, echo, sizeof(echo));
    if (n < 0) {
        perror("Reading Error: ");
    }
    echo[n] = '\0';
    printf("Client read %s\n", echo);
}
