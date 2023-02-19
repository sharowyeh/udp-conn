#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
   
#define PORT 65432
#define BUFSIZE 1024
   
int main(int argc, char* argv) {
    int socket_fd;
    char buffer[BUFSIZE];
    const char *msg = "Hello World!";
    struct sockaddr_in server_addr;
   
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        printf("open socket error");
        exit(1);
    }
   
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
       
    int msg_length;
    int server_length = sizeof(server_addr);
       
    msg_length = sendto(socket_fd, msg, strlen(msg), MSG_CONFIRM,
		    (const struct sockaddr *)&server_addr, server_length);
    if (msg_length < 0) {
	    printf("send to server error");
	    exit(1);
    }
    printf("message sent\n");
           
    msg_length = recvfrom(socket_fd, (char *)buffer, BUFSIZE, MSG_WAITALL,
		    (struct sockaddr *)&server_addr, &server_length);
    if (msg_length < 0) {
	    printf("receive response error");
	    exit(1);
    }
    printf("server resp: %s\n", buffer);
   
    close(socket_fd);
    return 0;
}
