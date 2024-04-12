# Communication between Server and Client
## Receiving the data from the server
Data is received and observed on the client-side with a delay of 100 milliseconds and also timeout for 100 milliseconds when data is not received. There are three TCP ports:
- Port 4001, there is a sequence of data received from the server with good frequency. For a 100-millisecond interval, there are many strings received and its values fluctuate from -5.0 to 5.0. The transition goes smoothly between peaks and troughs so peak-to-peak amplitude is from -5.0 to 5.0. The shape could be a sine wave signal.
- Port 4002, the data value is also from -5.0 to 5.0 like port 4001 but with lower frequency. In 100-millisecond intervals, one data is received and sometimes no data is received from the server. Peak-to-peak amplitude is also from -5.0 to 5.0 but the transition is sharp. It rises linearly from -5.0 to 5.0 then falls right away linearly to -5.0. Therefore, the shape is a triangular wave. 
- Port 4003, the frequency is realy low. There is data received every 100-millisecond or 200-millisecond interval. The amplitude of this signal is from 0.0 to 5.0 and there are only two values 0.0 and 5.0. The data changes instantly from 0.0 to 5.0 resulting in square pulses. The shape is a square wave. 

## How it works
### Client1 
First of all, One port is used to connect to get data from the server with TCP protocol. Therefore, it is much easier to build the code and test the code. The socket is created with supported libraries such as socket.h and inet.h and then connected to the server with port number and address. The socket is used with a timeout to make sure to get a response when no data is received from the server from 100 milliseconds.

``` c
struct timeval tv;
tv.tv_sec = 0;
tv.tv_usec = 100000;
setsockopt(*fd,SOL_SOCKET,SO_RCVTIMEO,(const char *) &tv,sizeof(tv));
```
For every loop with a 100-millisecond delay, the client will receive data with recv() function. If there is no data received from the server when reaches timeout, recv() will return -1. It will turn 0 when a server stops sending data. If everything works properly, it will return bytes and all data will be kept in buffer variable. Moreover, the strtok() function will break down into tokens based on delimiter '\n' and last one will be taken, it is also the most recent data. 

```c
int bytes_received = recv(fd, buffer, BUFFER_SIZE,0 );
// take the last byte based on delimiter '\n'
char * position = strtok(buffer,"\n");
        while(position != NULL){
            strncpy(value,position,sizeof(value));
            position = strtok(NULL,"\n");
        }
```

To make it comfortable to stop the program sufficiently, the signal is set to get the ctrl-c signal to stop the program.

```c
signal(SIGINT,sig_handler);
```
For get_data(int fd,char * value) function, it is possible to create a dynamic memory allocation for reading value from buffer and buffer has enough size for the data. FIONREAD request in ioctl(), I/O device control function is used to do this purpose.  However, array with fixed size still works well because the data has under 100 bytes for 100 milliseconds and its memory will be deallocated and array will crease after function completes its execution.

```c
 int bytes =0; 
ioctl(fd,FIONREAD,&bytes);
char * buffer =(char *) malloc(bytes);
int bytes_received = recv(fd, buffer, bytes, MSG_DONTWAIT);

// free after usage
 free(buffer);
```
To make sure it works well, every port will be tested and the data is received properly. All the data will be printed from the buffer first and then the value handled with timeout and with most recent data will be shown also. It is easy to make comparison that data is matched and meets the requirements.

### Client 2
Only the valid values from out3 are handled and "--" is ignored. To control the property in the server, the control protocol is useful. First of all, every field is set with unsigned 16-bit integer in an array so the server can read data as the whole protocol and it is also converted to network byte order(Big-Endian). 
Next, **Read Operation** is used to read data in the server but there is no information about frequency and amplitude properties provided but the object could be 1,2,3 from port1, port2, and port3. It is good to test read operations with some random numbers for properties increasing in the loop and monitor the standard output of the server side at the same time. 

```c    
    uint16_t  messange[3];
    messange[0] = htons(READ);
    messange[1] = htons(1);
    messange[2] = htons(randomNUmber);
    if (sendto(fd,messange, sizeof(messange), 0, (struct sockaddr *)&server, len_of_server) < 0) {
        printf("Error in connect  on port %d due to: %s\n",port,strerror(errno));
        close(fd);
        exit(1);
    }
```

In server side, the output shows the information such as object number, property and current value.

```bash
Client connected
...
1.170: amplitude=5000
...
1.255: frequency=500
1.256: no such property
...
```
After getting information on properties, the **WRITE operation** can be used. However, the frequency value 1 or 2 is not in the range from 50 to 2000 in the server.
```c 
uint16_t  messange[4];
messange[0] = htons(WRITE);
messange[1] = htons(1);
messange[2] = htons(255);
messange[3] = htons(1);

```
```bash
error,Value out of range, 1 not in [50, 2000]
...
```
After taking consideration of this value, there is an **assumption** that the value does **not need** to be in the network byte order(Big-Endian). In the server, value 1 or 2 will be 256 or 512 and it is near to the default value 500 in output1 and it is still in the range from 50 to 2000. Moreover, the frequency value is 500 in output1 and 250 in output2 so there is **another assumption** that **value 1 of frequency = 250 and value 2 of frequency = 500**. 


```bash
...
ok,frequency=256
ok,amplitude=8000
...
ok,frequency=512
ok,amplitude=4000
...
```
or 

```bash
...
ok,frequency=500
ok,amplitude=4000
ok,frequency=250
ok,amplitude=8000
...
```


### Build and Run
Makefile is used to compile the code. By default, it will compile and build both client1 and client2 if there is no argument in the make command. 
Moreover, to be more convenient, run and clean commands are added to run the executable file and clean both objects and executable files. For run command, client1 or client2 should be added as an argument to CLIENT.

```bash
make // build client1 and client2

make client1
make client2

make run CLIENT=client1
make run CLIENT=client2
make clean
```

## Conclusion
This is an interesting task, there is a combination of many things such as UPD and TCP protocol programming, and also how to solve problems from many different points. This is also very practical. Hopefully, there will be more similar tasks in the future. 