#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define BUF_SIZE 1000
#define QUEUE_SIZE 10

struct thread_data_t
{
	char s_in[BUF_SIZE];
	char s_out[BUF_SIZE];
	int s_connection_socket_descriptor;
};

void *ThreadBehavior(void *t_data)
{	
	pthread_detach(pthread_self());
	struct thread_data_t *th_data = (struct thread_data_t *)t_data;
	while(1) {
		//send(th_data->s_connection_socket_descriptor, th_data->s_out, BUF_SIZE);
		recv(th_data->s_connection_socket_descriptor, th_data->s_in, BUF_SIZE, 0);
		if(!strcmp(th_data->s_in, "//exit")) break;
		printf("%d ;; %s\n", th_data->s_connection_socket_descriptor, th_data->s_in);
	}
	close(th_data->s_connection_socket_descriptor);
	free(th_data);
	puts(th_data->s_in);
	pthread_exit(NULL);
}

void handleConnection(int connection_socket_descriptor) {
	pthread_t thread;
	struct thread_data_t *t_data;
	t_data = malloc(sizeof(struct thread_data_t));
	
	t_data->s_connection_socket_descriptor = connection_socket_descriptor;
	
	pthread_create(&thread, NULL, ThreadBehavior, (void *)t_data);
}

int main(int argc, char* argv[])
{
	int server_socket_descriptor;
	int connection_socket_descriptor;
	char reuse_addr_val = 1;
	struct sockaddr_in server_address;

	memset(&server_address, 0, sizeof(struct sockaddr));
	server_address.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &(server_address.sin_addr));
	server_address.sin_port = htons(1234);

	server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(server_socket_descriptor, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse_addr_val, sizeof(reuse_addr_val));
	bind(server_socket_descriptor, (struct sockaddr*)&server_address, sizeof(struct sockaddr));
	listen(server_socket_descriptor, QUEUE_SIZE);


	while(1) {
		connection_socket_descriptor = accept(server_socket_descriptor, NULL, NULL);
		handleConnection(connection_socket_descriptor);
	}
	
	close(server_socket_descriptor);
	return 0;
}
