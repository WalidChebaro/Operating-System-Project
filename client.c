#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#define PORT 4321
   
int main(int argc, char const *argv[]) 
{ 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    char line[1024];
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 
    while(1){
    // char path[1024];
    int dontRead = 1;
    read(sock,line,1024);//get current directory from server
    printf("CLIENT SHELL -:- %s:", line);
    fgets(line,sizeof(line),stdin);//read from stdinof user the command
    if(line[0] == 'c' && line[1] == 'd')dontRead = 0;//if command is cd
    if(line[0] == '.' && line[1] == '/')dontRead = 0;//if command starts with ./
    if(line[0] == 'r' && line[1] == 'm')dontRead = 0;//if command is rm
    if(line[0] == 'm' && line[1] == 'k' && line[2] == 'd' && line[3] == 'i' && line[4] == 'r')dontRead = 0;
    //if command is mkdir
    send(sock , line , strlen(line) , 0 );//send command to server
    if(dontRead == 1)read(sock,line,1024);//read input from server
    if(dontRead == 1)printf("%s\n", line);
    }
    return 0; 
} 