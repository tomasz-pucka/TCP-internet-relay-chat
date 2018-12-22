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

#define BUF_SIZE 4096
#define QUEUE_SIZE 10
#define NICK_SIZE 24

struct users {
	int descriptor;
	char nick[NICK_SIZE];
	struct users *next;
};

struct rooms {
	unsigned int number;
	struct users *head;
	struct rooms *next;
};

struct rooms *rooms_list = NULL;

struct thread_data_t
{
	char s_in[BUF_SIZE];
	char s_out[BUF_SIZE];
	int s_connection_desc;
};

void printRooms(struct rooms * room) {
	if (room == NULL) return;
	while (room) {
		printf("%d  ", room->number);
		room = room->next;
	}
	printf("\n");
}

void addRoomFront(struct rooms ** head, unsigned int nr) {
	struct rooms *new_room = (struct rooms *) malloc(sizeof(struct rooms));
	new_room->number = nr;
	new_room->next = *head;
	new_room->head = NULL;
	*head = new_room;
}

void removeRoom(struct rooms ** head, unsigned int nr) {
	struct rooms *temp = *head;
	struct rooms *del_room;
	if (temp->number == nr) {
		*head = temp->next;
		free(temp);
	}
	else {
		while (temp->next->number != nr) temp = temp->next;
		del_room = temp->next;
		temp->next = del_room->next;
		free(del_room);
	}
}

void printUsersInRoom(struct rooms * room, unsigned int nr) {
	struct users *user;
	while (room && room->number != nr) room = room->next;
	if (room == NULL) return;
	user = room->head;
	printf("#%d: ", nr);
	while (user) {
		printf("%d ; %s\t", user->descriptor, user->nick);
		user = user->next;
	}
	printf("\n");
}

void addUserToRoomFront(struct rooms ** head, unsigned int nr, int desc, const char *nick) {
	struct rooms *room = *head;
	while (room && room->number != nr) room = room->next;
	if (room == NULL) {
		addRoomFront(head, nr);
		room = *head;
	}
	struct users *new_user = (struct users *) malloc(sizeof(struct users));
	new_user->descriptor = desc;
	strcpy(new_user->nick, nick);
	new_user->next = room->head;
	room->head = new_user;
}

void removeUserFromRoom(struct rooms ** head, unsigned int nr, int desc) {
	struct rooms *room = *head;
	while (room->number != nr) room = room->next;
	struct users *temp = room->head;
	if (temp->descriptor == desc) {
		room->head = temp->next;
		free(temp);
		if (room->head == NULL) removeRoom(head, nr);
	}
	else {
		while (temp->next->descriptor != desc) temp = temp->next;
		struct users *del_user = temp->next;
		temp->next = del_user->next;
		free(del_user);
	}

}


void *ThreadBehavior(void *t_data)
{	
	pthread_detach(pthread_self());
	struct thread_data_t *th_data = (struct thread_data_t *)t_data;
	while(1) {
		//send(th_data->s_connection_desc, th_data->s_out, BUF_SIZE);
		recv(th_data->s_connection_desc, th_data->s_in, BUF_SIZE, 0);
		if(!strcmp(th_data->s_in, "//exit")) break;
		printf("%d ;; %s\n", th_data->s_connection_desc, th_data->s_in);
	}
	close(th_data->s_connection_desc);
	free(th_data);
	pthread_exit(NULL);
}

void handleConnection(int connection_desc) {
	pthread_t thread;
	struct thread_data_t *t_data;
	t_data = malloc(sizeof(struct thread_data_t));
	t_data->s_connection_desc = connection_desc;
	pthread_create(&thread, NULL, ThreadBehavior, (void *)t_data);
}

int main(int argc, char* argv[])
{
	int server_socket_descriptor;
	int connection_desc;
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
		connection_desc = accept(server_socket_descriptor, NULL, NULL);
		handleConnection(connection_desc);
	}
	
	close(server_socket_descriptor);
	return 0;
}
