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

#define QUEUE_SIZE 5

struct thread_data_t
{
	char s_buf[100];
	int s_connection_socket_descriptor;
};

//funkcja opisującą zachowanie wątku - musi przyjmować argument typu (void *) i zwracać (void *)
void *ThreadBehavior(void *t_data)
{
	pthread_detach(pthread_self());
	struct thread_data_t *th_data = (struct thread_data_t*)t_data;
	//dostęp do pól struktury: (*th_data).pole
	write((*th_data).s_connection_socket_descriptor, &((*th_data).s_buf), 100);
	printf("%s\n", (*th_data).s_buf);
	pthread_exit(NULL);
}

void handleConnection(int connection_socket_descriptor) {
	pthread_t thread1;
	
	//struct thread_data_t *t_data = malloc(sizeof(struct thread_data_t));
	//strcpy(&(t_data->s_buf), "rererere123");
	//t_data->s_connection_socket_descriptor = connection_socket_descriptor;
	
	struct thread_data_t t_data;
	t_data.s_connection_socket_descriptor = connection_socket_descriptor;
	strcpy(t_data.s_buf, "rerer123");
	
	pthread_create(&thread1, NULL, ThreadBehavior, (void *)&t_data);
	//free(t_data);
	sleep(2);
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


	while(1)
	{
		connection_socket_descriptor = accept(server_socket_descriptor, NULL, NULL);
		handleConnection(connection_socket_descriptor);
		close(connection_socket_descriptor);
	}

	close(server_socket_descriptor);
	return(0);
}
