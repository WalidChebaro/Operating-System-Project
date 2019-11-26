#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

#define PORT 4321

static char *myStrDup (char *str) {  //just to make a copy from a string
    char *other = malloc (strlen (str) + 1);
    if (other != NULL)
        strcpy (other, str);
    return other;
}
int main(){
int server_fd, new_socket, valread; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
    char entry[1024] = {0}; 
    char *hello = "Hello from server"; 
       
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    // Forcefully attaching socket to the port 4321 
    // if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
    //                                               &opt, sizeof(opt))) 
    // { 
    //     perror("setsockopt"); 
    //     exit(EXIT_FAILURE); 
    // } 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( PORT ); 
       
    // Forcefully attaching socket to the port 4321 
    if (bind(server_fd, (struct sockaddr *)&address,  
                                 sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    if (listen(server_fd, 3) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,  
                       (socklen_t*)&addrlen))<0) 
    { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    }     
    while(1){
        char cwd[PATH_MAX];
        // printf(RED"SHELL:~#"YEL" %s:\t"RESET,getcwd(cwd, sizeof(cwd)));
        // read(new_socket, size , sizeof());
        char entry[1024] = {0}; 
        send(new_socket , getcwd(cwd, sizeof(cwd)) , 1024 , 0 );
        read( new_socket , entry, 1024);
        entry[strlen(entry)-1] = '\0';
        char *argv[100];
        int argc = 0;
        char *str = strtok (entry, " ");
        while (str != NULL) {
            argv[argc++] = myStrDup (str);
            str = strtok (NULL, " ");
        }
        argv[argc] = NULL;
        int j =0 , i = 0, count = 0,initCount , ptr = 0 , ppipe = 0;
        //counting how many pipes do we have so that we know how many processes we need to fork
        for(i=0;i<argc;i++)if(strcmp(argv[i],"|")==0)count++;
        while(ppipe<argc && strcmp(argv[ppipe],"|")!=0)ppipe++;
        
        if(ppipe == 0){printf(RED"error\n"RESET);break;}//return break to continue
        char * temp[100];
        /* now we are storing each part in the temp */
        for(i=ptr;i < ppipe;i++)temp[j++] = argv[i];
        temp[j]=NULL;
        i=0;j=0;ppipe++;ptr = ppipe;
        if(ppipe == argc){
            printf(RED"pipe cannot be the last entry\n"RESET);
            continue;
        }

        //if there is not any pipes, excute directly
        if(count==0){
            //the next 2 lines are for editing the path
            if(strcmp(temp[0],"cd")==0){chdir(temp[1]);continue;}
            pid_t pidd=fork();
            if(pidd==0){
                dup2(new_socket, 1);
                execvp(temp[0],temp);
            }
            else if(pidd>0)wait(NULL);
            }
        if(count == 1){
            pid_t pid = fork();
            if(pid == 0){
                int fd[2];
                pipe(fd);
                pid_t pi = fork();
                if(pi == 0){
                    close(fd[0]);
                    close(1);
                    dup(fd[1]);
                    execvp(temp[0],temp);
                }
                if(pi > 0){
                    dup2(new_socket, 1);
                    close(fd[1]);
                    close(0);
                    dup(fd[0]);
                    wait(NULL);
                    char * temp[100];
                    int z,w=0;
                    while(count>0){
                        ptr = ppipe;
                        while(ppipe<argc && strcmp(argv[ppipe++],"|")!=0);
                        count--;
                        }
                    for(z=ptr; z < ppipe;z++)temp[w++] = argv[z];
                    temp[w]=NULL;                
                    execvp(temp[0],temp);

                }
            }
            else if(pid > 0){
                wait(NULL);
            }
        }
        if(count == 2){
            pid_t p = fork();
            if(p == 0){
                int fd[2];
                pipe(fd);
                pid_t pi = fork();
                if(pi == 0){
                    close(fd[0]);
                    close(1);
                    dup(fd[1]);
                    int pd[2];
                    pipe(pd);
                    pid_t pid = fork();
                    if(pid == 0){
                        //last child executes first command
                        close(pd[0]);
                        close(1);
                        dup(pd[1]);
                        execvp(temp[0],temp);
                    }
                    else if (pid > 0){
                        //2rd last child executes 2nd command
                        close(pd[1]);
                        close(0);
                        dup(pd[0]);
                        wait(NULL);
                        char * temp[100];
                        int z,w=0;
                        count--;
                        while(count>0){
                            ptr = ppipe;
                            while(ppipe<argc && strcmp(argv[ppipe++],"|")!=0);
                            count--;
                            }
                        for(z=ptr; z < ppipe-1;z++)temp[w++] = argv[z];
                        temp[w]=NULL;                
                        execvp(temp[0],temp);
                    }
                }
                //main's child executes last and executes third command
                else if (pi > 0){
                    dup2(new_socket, 1);
                    close(fd[1]);
                    close(0);
                    dup(fd[0]);
                    wait(NULL);
                    char * temp[100];
                    int z,w=0;
                    while(count>0){
                        ptr = ppipe;
                        while(ppipe<argc && strcmp(argv[ppipe++],"|")!=0);
                        count--;
                        }
                    for(z=ptr; z < ppipe;z++)temp[w++] = argv[z];
                    temp[w]=NULL;                
                    execvp(temp[0],temp);
                }
            }
            //main waits untill all childer execute
            else if(p > 0) wait(NULL);
        }
    }
    return 0;
}