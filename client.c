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

	int main() {
		char buf[100];
		int connection_socket_descriptor;
		struct sockaddr_in server_address;

		memset(&server_address, 0, sizeof(struct sockaddr));
		server_address.sin_family = AF_INET;
		inet_pton(AF_INET, "127.0.0.1", &(server_address.sin_addr));
		server_address.sin_port = htons(1234);
		
		while(1) {
			connection_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
			connect(connection_socket_descriptor, (struct sockaddr*)&server_address, sizeof(server_address));
			read(connection_socket_descriptor, buf, 100);
			printf("%s\n", buf);  
		}
		close(connection_socket_descriptor);
		return 0;
	}
