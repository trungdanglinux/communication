#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#define BUFFER_SIZE 2000
int fd;

void sig_handler(int signum) {
    printf("\nStop receiving data from server\n");
    close(fd);
    exit(0);
}
int main(){
    char value[5];
    char buffer[BUFFER_SIZE];
    size_t bytes_received;
    // Initialize the socket 
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(4001);
    addr.sin_addr.s_addr= inet_addr("0.0.0.0");
    fd = socket(AF_INET,SOCK_STREAM,0);
    if(fd < 0){
        printf("Error to create socket due to: %s\n",strerror(errno));
    }
    //conect socket file address to file descriptor 
    int connecting = connect(fd,(struct sockaddr * )&addr,sizeof( addr));
    if (connecting < 0){
        printf("Error in connect due to: %s\n",strerror(errno));
        close(fd);
        return 1;
    }
    //Set interupt signal
    if(signal(SIGINT,sig_handler) == SIG_ERR){
        printf("Error to signal handler due to: %s\n",strerror(errno));
        exit(-1);
    }
    while((bytes_received = recv(fd, buffer, BUFFER_SIZE, 0))>0) {
        if (buffer == NULL ) {
            strcpy(value,"--");       
        }else {
            printf("Received: %s", buffer);
            if(bytes_received <= 5){
                strcpy(value,buffer);
                value[bytes_received-1] ='\0';
            } 
            else{
                char * position = strtok(buffer,"\n");
                while(position != NULL){
                    strncpy(value,position,sizeof(value));
                    position = strtok(NULL,"\n");
                }
            }
        }
        printf("Recent Received: %s\n", value);
        memset(buffer, '\0', sizeof(buffer));
        memset(value, '\0', sizeof(value));
        usleep(100000);
    }
    close(fd);
    return 0;
}