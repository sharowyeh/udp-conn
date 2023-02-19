#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 65432
#define BUFSIZE 1024

// just a simple udp echo service
int main(int argc, char* argv) {
	int socket_fd;
	int client_length;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	struct hostent *host_info;
	char buf[BUFSIZE];
	char *host_addr_str;
	int optval;
	int msg_length;

	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd < 0) {
		printf("open socket error");
		exit(1);
	}

	optval = 1;
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR,
			(const void *)&optval, sizeof(int));

	// ok with bzero but prefer memset more generic for my usage
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons((unsigned short)PORT);

	if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		printf("bind fd error");
		exit(1);
	}

	// main loop
	client_length = sizeof(client_addr);
	int listening = 1;
	while (listening) {
		memset(buf, 0, BUFSIZE);
		msg_length = recvfrom(socket_fd, buf, BUFSIZE, 0,
				(struct sockaddr *)&client_addr, &client_length);
		if (msg_length < 0) {
			printf("receive error");
			exit(1);
		}

		host_info = gethostbyaddr((const char *)&client_addr.sin_addr.s_addr,
				sizeof(client_addr.sin_addr.s_addr), AF_INET);
		if (host_info == NULL) {
			printf("get host from client error");
			exit(1);
		}

		host_addr_str = inet_ntoa(client_addr.sin_addr);
		if (host_addr_str == NULL) {
			printf("get addr from client error");
			exit(1);
		}

		printf("client: %s(%s)\n", host_info->h_name, host_addr_str);
		printf("data: %d/%d bytes: %s\n", strlen(buf), msg_length, buf);

		msg_length = sendto(socket_fd, buf, strlen(buf), 0,
				(struct sockaddr *)&client_addr, client_length);
		if (msg_length < 0) {
			printf("send to client error");
			exit(1);
		}

	}
}

