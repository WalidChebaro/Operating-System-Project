/* Code that shows how to create a multi-threaded server in C.
   The server uses a IPv4 and TCP as a network protocol.
   This is an ECHO server that spawns a new thread for every client and
   echos an upper case representation of the received message.
   Proper error handling is omitted due to brevity.
*/
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#define SERV_PORT 8082

#define MAXLINE 100
#define BACKLOG 3

void *server_handler(void *);
void convertToUpper(char *);

typedef struct thread_data
{
    int id;
    int conn;
} THREAD_DATA;

int main(int argc, char *argv[])
{
    char buff[MAXLINE];
    int server_socket = 0; // server socket used to listen and accept incoming connections
    int client_socket = 0; // client socket used to communicate with a connected client
    int new_socket;
    struct sockaddr_in servaddr; // a struct that holds information about internet address

    /* Create an unbound socket and return a file descriptor
       int socket(int family, int type, int protocol)
    */
    server_socket = socket(AF_INET, SOCK_STREAM, 0); // IPV4, STREAM, TCP
    if (server_socket < 0)
        exit(1);

    servaddr.sin_family = AF_INET;                // set the domain
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // set the IP without knowing the IP of the running system
    servaddr.sin_port = htons(SERV_PORT);         // set the port number

    /* Assign a local socket address (servaddr) to a socket identified by a descriptor (server_socket)
       bind(int sockfd, struct sockaddr *my_addr, int addrlen)
    */
    if (bind(server_socket, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("binding error");
        exit(1);
    }
    /* Mark the socket to be used to accept connection requests
       Set the maximum size of the pending connection queue
    */
    if (listen(server_socket, BACKLOG) < 0)
    {
        exit(1);
    }

    if ((client_socket = accept(server_socket, (struct sockaddr *)NULL, NULL)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    printf("Client connected\n");

    /* Read message from client */
    int n = read(client_socket, buff, sizeof(buff));
    if (n < 0)
        perror("Reading Error: ");
    else
        buff[n] = '\0';

    printf("Server read %s\n", buff);
    execlp("./shell_projec.c",buff,NULL);

    if (write(client_socket, buff, strlen(buff)) < 0)
        perror("Writing Error: ");
    return 0;
}
