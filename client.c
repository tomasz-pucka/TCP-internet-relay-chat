#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>

#define BUF_SIZE 4096

int connection_socket_descriptor;
char text_buf[BUF_SIZE];
char in_buf[BUF_SIZE];

void *printIncomingMessagesThread() {
	while(1){
		recv(connection_socket_descriptor, in_buf, BUF_SIZE, 0);
		printf("%s\n", in_buf);
	}
}

int main() {
	pthread_t thread;
	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(struct sockaddr));
	server_address.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &(server_address.sin_addr));
	server_address.sin_port = htons(1234);
	connection_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	connect(connection_socket_descriptor, (struct sockaddr*)&server_address, sizeof(server_address));
	system("clear");
	printf("type room;nick\n");
	fgets(text_buf, BUF_SIZE, stdin);
	text_buf[strcspn(text_buf, "\n")] = 0;
	send(connection_socket_descriptor, text_buf, BUF_SIZE, 0); // room;nick
	pthread_create(&thread, NULL, printIncomingMessagesThread, NULL);
	while(1) {
		fgets(text_buf, BUF_SIZE, stdin);
		text_buf[strcspn(text_buf, "\n")] = 0;
		send(connection_socket_descriptor, text_buf, BUF_SIZE, 0);
		if(!strcmp(text_buf, ";;exit")) break;
	}
	close(connection_socket_descriptor);
	return 0;
}
