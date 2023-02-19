#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <math.h> /*may need -lm to c compiler for libm.a*/
#include <fcntl.h>

#define BUFSIZE 1024
#define BASE_MS 500 /*expoential backoff wait base(ms)*/
#define MULTIPLIER 2 /*multiplier for waiting time*/
#define MAX_RETRY 10
#define MAX_WAIT_MS 8000

int main(int argc, char **argv) {
	if (argc != 3) {
		printf("usage: client <ip> <port>\n");
		exit(1);
	}
	char *host_addr_str = argv[1];
	int port_no = atoi(argv[2]);
	char msg[BUFSIZE] = "Hello World!";
	// if need user input, but Hello World is awesome
	//fgets(msg, BUFSIZE, stdin);

	int socket_fd;
	char buffer[BUFSIZE];
	struct sockaddr_in server_addr;
	struct hostent *host_info;

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port_no);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	host_info = gethostbyname(host_addr_str);
	if (host_info == NULL) {
		printf("get host error");
		exit(1);
	}
	memcpy((char *)&server_addr.sin_addr.s_addr, (char *)host_info->h_addr, host_info->h_length);

	int msg_length;
	int server_length = sizeof(server_addr);

	useconds_t wait_time = BASE_MS * 1000;
	int retry = 0;
	int is_timeout = 1;
	while (retry < MAX_RETRY) {
		if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			printf("open socket error");
			exit(1);
		}

		/* use non-blocking if really want to handle thread and wait by ourselfs */
		//fcntl(socket_fd, F_SETFL, O_NONBLOCK);

		// calc expoential backoff interval
		wait_time = BASE_MS * 1000 * pow(MULTIPLIER, retry);
		wait_time = (wait_time > MAX_WAIT_MS * 1000) ? MAX_WAIT_MS * 1000 : wait_time;
		struct timeval tv;
		tv.tv_sec = wait_time / 1000000;
		tv.tv_usec = wait_time % 1000000;

		// change send timeout opt for blocked socket
		if (setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
			printf("socket opt error");
			exit(1);
		}

		// send message
		msg_length = sendto(socket_fd, msg, strlen(msg), 0,
				(const struct sockaddr *)&server_addr, server_length);
		if (msg_length >= 0) {
			printf("message sent\n");

			// change receive timeout opt to blocked socket
			if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
				printf("socket opt error");
				exit(1);
			}

			// clean buffer and get echo string
			memset(buffer, 0, BUFSIZE);
			msg_length = recvfrom(socket_fd, (char *)buffer, BUFSIZE, 0,
					(struct sockaddr *)&server_addr, &server_length);
			if (msg_length < 0) {
				printf("receive response error\n");
			} else {
				printf("server resp: %s\n", buffer);
			}
		}

		//fcntl(socket_fd, F_SETFL, ~O_NONBLOCK & fcntl(socket_fd, F_GETFL));
		close(socket_fd);

		if (msg_length >= 0) {
			is_timeout = 0;
			break;
		}

		printf("expo wait %d ms, retry: %d\n", wait_time / 1000, retry);
		retry++;
	} // while -END

	return is_timeout;
}

