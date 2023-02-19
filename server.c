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
int main(int argc, char **argv) {
	// specify port number or just use default 65432
	unsigned short port_no = PORT;
	if (argc > 1) {
		port_no = atoi(argv[1]);
	}
	printf("use port %d\n", port_no);

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

	// ok if bzero or bxxx, but I like more generic memset or memxxx if not specify
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port_no);

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

		char host_name[128] = {0};
		host_info = gethostbyaddr((const char *)&client_addr.sin_addr.s_addr,
				sizeof(client_addr.sin_addr.s_addr), AF_INET);
		// not all clients provide proper info, like env behind virtualization
		if (host_info != NULL) {
			memcpy(host_name, host_info->h_name, strlen(host_info->h_name));
			//TODO: cpy length better from min(a,b)
		}

		host_addr_str = inet_ntoa(client_addr.sin_addr);
		if (host_addr_str == NULL) {
			printf("get addr from client error");
			exit(1);
		}

		printf("client: %s(%s)\n", host_name, host_addr_str);
		printf("data: %d/%d bytes: %s\n", strlen(buf), msg_length, buf);

		msg_length = sendto(socket_fd, buf, strlen(buf), 0,
				(struct sockaddr *)&client_addr, client_length);
		if (msg_length < 0) {
			printf("send to client error");
			exit(1);
		}

	}
}

