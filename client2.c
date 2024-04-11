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

#define BUFFER_SIZE 1024
#define PORT0 4000
#define PORT1 4001
#define PORT2 4002
#define PORT3 4003
#define TCP 0
#define UDP 1
#define READ 1
#define WRITE 2
#define OUT1 1
#define FREQ 255 // frequency property in server
#define AMPLI 170 // amplitude property in server


int fd0,fd1,fd2,fd3;
int loop = 1;
char IP[16] = "0.0.0.0";
void sig_handler(int signum) {
    printf("\nStop receiving data from server\n");
    close(fd1);
    close(fd2);
    close(fd3);
    exit(0);
}
int init_socket(int * fd,int protocol){
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 20000;
    if(protocol == TCP){   
        *fd = socket(AF_INET,SOCK_STREAM,0);
        if(*fd < 0){
            printf("Error to create socket due to: %s\n",strerror(errno));
            return 1;
        }
        setsockopt(*fd,SOL_SOCKET,SO_RCVTIMEO,(const char *) &tv,sizeof(tv));
    }else if(protocol == UDP){
        *fd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
        if(*fd < 0){
            printf("Error to create socket due to: %s\n",strerror(errno));
            return 1;
        }
    }
    return 0;
}
int connect_socket(int fd,int port , char* ip ){
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
    int bytes_received = recv(fd, buffer, BUFFER_SIZE,MSG_DONTWAIT );
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

void send_control(int fd,int port , char* ip, uint16_t operation, uint16_t object,uint16_t property,uint16_t value){
    struct sockaddr_in server,client;
    int len_of_server= sizeof(server);
    socklen_t client_addr_len = sizeof(client);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);
    //Pack data in MSB order
    uint16_t  message[4];
    message[0] = htons(operation);
    message[1] = htons(object);
    message[2] = htons(property);
    message[3] = htons(value);
    if (sendto(fd,message, sizeof(message), 0, (struct sockaddr *)&server, len_of_server) < 0) {
        printf("Error in connect  on port %d due to: %s\n",port,strerror(errno));
        close(fd);
        exit(1);
    }
}
void set_value(int value){
    uint16_t freq = 0;
    uint16_t ampli = 0;
    if(value >= 3){
        freq = ntohs(1); // value from 50 - 2000
        ampli = 8000;
        send_control(fd0,PORT0,IP,WRITE,OUT1,FREQ,freq);
        send_control(fd0,PORT0,IP,WRITE,OUT1,AMPLI,ampli);
    }else{
        freq =ntohs(2);
        ampli = 4000;
        send_control(fd0,PORT0,IP,WRITE,OUT1,FREQ,freq);
        send_control(fd0,PORT0,IP,WRITE,OUT1,AMPLI,ampli);
    }
}
void print(int64_t timestamp, char * out1, char *out2, char *out3){
    printf("{\"timestamp\": %lld, \"out1\": \"%s\", \"out2\": \"%s\", \"out3\": \"%s\"}\n",
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
    
    //Initialize the socket tcp and udp
    if(init_socket(&fd1,TCP) != 0 ||init_socket(&fd2,TCP) != 0 || 
                                init_socket(&fd3,TCP) != 0 || init_socket(&fd0,UDP) != 0 ){
        return 1;
    }
    //conect socket file to server 
    if(connect_socket(fd1,PORT1,IP) || connect_socket(fd2,PORT2,IP)|| connect_socket(fd3,PORT3,IP)){
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
        usleep(20000);
        int64_t timeStamp = get_timestamp();
        get_data(fd1, output1);
        get_data(fd2, output2);
        get_data(fd3, output3);
        //ignore the "--" value 
        if (strcmp(output3,"--") != 0 ) {
            int out3 = atoi(output3);
            set_value(out3);
        }    
        print(timeStamp, output1, output2, output3);
        memset(output1, '\0', sizeof(output1));
        memset(output2, '\0', sizeof(output2));
        memset(output3, '\0', sizeof(output3));
    }
    close(fd0);
    close(fd1);
    close(fd2);
    close(fd3);
    return 0;
}