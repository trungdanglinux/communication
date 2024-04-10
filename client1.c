#define _XOPEN_SOURCE 500
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
#include <time.h>
#define BUFFER_SIZE 1024
#define PORT1 4001
#define PORT2 4002
#define PORT3 4003

int fd1,fd2,fd3;
int loop = 1;
void sig_handler(int signum) {
    printf("\nStop receiving data from server\n");
    close(fd1);
    close(fd2);
    close(fd3);
    exit(0);
}
int init_socket(int * fd){
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    *fd = socket(AF_INET,SOCK_STREAM,0);
    if(*fd < 0){
        printf("Error to create socket due to: %s\n",strerror(errno));
        return 1;
    }
    setsockopt(*fd,SOL_SOCKET,SO_RCVTIMEO,(const char *) &tv,sizeof(tv));
    return 0;
}
int connect_socket(int fd,int port , char* ip){
    struct sockaddr_in sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    sock_addr.sin_addr.s_addr= inet_addr(ip);
    int connecting = connect(fd,(struct sockaddr * )&sock_addr,sizeof( sock_addr));
    if (connecting < 0){
        printf("Error in connect  on port %d due to: %s\n",port,strerror(errno));
        close(fd);
        return 1;
    }   
    return 0;
}

void get_data(int fd,char * value){
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(fd, buffer, BUFFER_SIZE,0 );
    if (bytes_received == 0) {
        loop = 0;
        printf("Server closed connection\n"); 
    } else if (bytes_received < 0 ){
        if (errno == EAGAIN || errno == EWOULDBLOCK) strcpy(value,"--");
        else printf("Error in receiving data from server");  
    }else {
        buffer[bytes_received] ='\0';
        char * position = strtok(buffer,"\n");
        while(position != NULL){
            strncpy(value,position,sizeof(value));
            position = strtok(NULL,"\n");
        }
    }
    memset(buffer, '\0', sizeof(buffer));    
}

void print(int64_t timestamp, char * out1, char *out2, char *out3){
    // THIS IS FROM THIRD-PARTY LIBRARY, NOT C STANDARD LIBRARY 
    // json_object *obj = json_object_new_object();
    // json_object_object_add(obj, "timestamp", json_object_new_int64(timestamp));
    // json_object_object_add(obj, "out1", json_object_new_string(out1));
    // json_object_object_add(obj, "out2", json_object_new_string(out2));
    // json_object_object_add(obj, "out3", json_object_new_string(out3));
    
    //Packing data
    printf("{\"timestamp\":%lld,\"out1\":\"%s\",\"out2\":\"%s\",\"out3\":\"%s\"}\n",
                             timestamp, out1, out2, out3 );

}
int64_t get_timestamp(){
    struct timeval measure;
    int64_t  timestamp;
    gettimeofday(&measure, NULL);
    timestamp = (int64_t)measure.tv_sec * 1000LL + (int64_t)measure.tv_usec / 1000LL;
    return timestamp;
}
int main(){
    char output1[5],output2[5],output3[5];
    char ip[16] = "0.0.0.0";
  

    struct timespec ts;
    ts.tv_sec =0;
    ts.tv_nsec = 100000000;

    // Initialize the socket 
    if(init_socket(&fd1) != 0 ||init_socket(&fd2) != 0 || init_socket(&fd3) != 0  ){
        return 1;
    }
    //conect socket file address to file descriptor 
    if(connect_socket(fd1,PORT1,ip) || connect_socket(fd2,PORT2,ip)|| connect_socket(fd3,PORT3,ip)){
        return 1;
    }

    //Set interupt signal
    if(signal(SIGINT,sig_handler) == SIG_ERR){
        printf("Error to signal handler due to: %s\n",strerror(errno));
        exit(-1);
    }
    //Get data
    while (loop)
    {
        get_data(fd1, output1);
        get_data(fd2, output2);
        get_data(fd3, output3);
        print(get_timestamp(), output1, output2, output3);
        usleep(100000);
        memset(output1, '\0', sizeof(output1));
        memset(output2, '\0', sizeof(output2));
        memset(output3, '\0', sizeof(output3));
    }
    close(fd1);
    close(fd2);
    close(fd3);
    return 0;
}