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
#include <errno.h>

#define BUF_SIZE 4096
#define QUEUE_SIZE 100
#define NICK_SIZE 24

//nick nie moze zawierac znaku ";"
//lista nickow przesylana w formie ";;nick01;nick02;nick03;"
//lista numerow pokojow przesylana raz przed wybraniem pokoju przez klienta w formie ";;1;2;3;"
//klient rozlacza sie wysylajac wiadomosc o tresci ";;exit"

char ip_addr[200] = "127.0.0.1";
int port_nr = 1234;
char accept_error = 0, recv_error = 0, send_error = 0;

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

struct client_main_thread_data {
	int s_connection_desc;
};

/*struct nicks_sender_thread_data {
	unsigned int number;
	int descriptor;
	char exit;
};*/

unsigned int *getRooms(struct rooms * room, unsigned int *rooms_counter) {
	unsigned int *rooms_arr = NULL;
	*rooms_counter = 0;
	while (room) {
		(*rooms_counter)++;
		rooms_arr = (unsigned int *)realloc(rooms_arr, sizeof(unsigned int)*(*rooms_counter));
		rooms_arr[(*rooms_counter) - 1] = room->number;
		room = room->next;
	}
	return rooms_arr;
}

char *getRoomsStr(struct rooms * room) {
	char *rooms_str, room_number_str[10];
	unsigned int rooms_str_len = 3;
	rooms_str = (char *)malloc(3*sizeof(char));
	strcpy(rooms_str, ";;");
	while (room) {
		sprintf(room_number_str, "%d", room->number);
		rooms_str_len += strlen(room_number_str) + 1;
		rooms_str = (char *)realloc(rooms_str, sizeof(char)*(rooms_str_len));
		strcat(rooms_str, room_number_str);
		strcat(rooms_str, ";");
		room = room->next;
	}
	return rooms_str;
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
		printf("%d: %s    ", user->descriptor, user->nick);
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

int *getOtherUsersInRoom(struct rooms * room, unsigned int nr, int sender, unsigned int *users_counter) {
	int *users_arr = NULL;
	struct users *user;
	*users_counter = 0;
	while (room && room->number != nr) room = room->next;
	if (room == NULL) return NULL;
	user = room->head;
	while (user) {
		if (user->descriptor != sender) {
			(*users_counter)++;
			users_arr = (int *)realloc(users_arr, sizeof(int)*(*users_counter));
			users_arr[*users_counter - 1] = user->descriptor;
		}
		user = user->next;
	}
	return users_arr;
}

char *getOtherNicksInRoom(struct rooms * room, unsigned int nr, int sender) {
	char *nicks_str;
	unsigned int nicks_str_len = 3;
	nicks_str = (char *)malloc(3*sizeof(char));
	strcpy(nicks_str, ";;");
	struct users *user;
	while (room && room->number != nr) room = room->next;
	if (room == NULL) return NULL;
	user = room->head;
	while (user) {
		if (user->descriptor != sender) {
			nicks_str_len += strlen(user->nick) + 1;
			nicks_str = (char *)realloc(nicks_str, sizeof(char)*(nicks_str_len));
			strcat(nicks_str, user->nick);
			strcat(nicks_str, ";");
		}
		user = user->next;
	}
	return nicks_str;
}

/*void *nicksSenderThread(void *t_data) {
	char *nicks_str;
	struct nicks_sender_thread_data *th_data = (struct nicks_sender_thread_data *)t_data;
	while(!th_data->exit) {
		free(nicks_str);
		nicks_str = getOtherNicksInRoom(rooms_list, th_data->number, th_data->descriptor);
		if (nicks_str == NULL) break;
		if (th_data->exit || send(th_data->descriptor, nicks_str, BUF_SIZE, 0) == -1) break;
		sleep(2);
	}
	free(nicks_str);
	pthread_exit(NULL);
}*/

void *printUsersInRoomsThread() {
	pthread_detach(pthread_self());
	unsigned int *rooms_arr = NULL, rooms_arr_len, i;
	while (1) {
		free(rooms_arr);
		rooms_arr = getRooms(rooms_list, &rooms_arr_len);
		system("clear");
		printf("IP: %s, PORT: %d\n", ip_addr, port_nr);
		for (i = 0; i < rooms_arr_len; i++) {
			printUsersInRoom(rooms_list, rooms_arr[i]);
		}
		if(accept_error) printf("\nACCEPT ERROR!\n");
		if(recv_error) printf("\nRECEIVE ERROR!\n");
		if(send_error) printf("\nSEND ERROR!\n");
		sleep(1);
	}
	pthread_exit(NULL);
}

void *clientThread(void *t_data)
{
	pthread_detach(pthread_self());
	char text_buf[BUF_SIZE + 1], message[BUF_SIZE + NICK_SIZE + 1], header[NICK_SIZE + 1];
	char *str_room_number, *nick, *rooms_str, *nicks_str;
	int *other_users = NULL, i, recv_code, send_code;
	unsigned int other_users_len = 0, room_number;
	struct client_main_thread_data *th_data = (struct client_main_thread_data *)t_data;
	rooms_str = getRoomsStr(rooms_list);
	send_code = send(th_data->s_connection_desc, rooms_str, BUF_SIZE, 0);
	free(rooms_str);
	if (send_code == -1) goto clientExitRoutine;
	recv_code = recv(th_data->s_connection_desc, text_buf, BUF_SIZE, 0); // receive "room;nick" string from client
	if (recv_code == -1) goto clientExitRoutine;
	str_room_number = strtok_r(text_buf, ";", &nick); // room;nick divider
	room_number = atoi(str_room_number);
	addUserToRoomFront(&rooms_list, room_number, th_data->s_connection_desc, nick);
	nicks_str = getOtherNicksInRoom(rooms_list, room_number, th_data->s_connection_desc);
	send_code = send(th_data->s_connection_desc, nicks_str, BUF_SIZE, 0);
	free(nicks_str);
	if (send_code == -1) send_error = 1;
	other_users = getOtherUsersInRoom(rooms_list, room_number, th_data->s_connection_desc, &other_users_len);
	strcpy(header, nick);
	strcat(header, ";");
	strcpy(message, ";;");
	strcat(message, header);
	strcat(message, "join");
	for (i = 0; i < other_users_len; i++) { 
		send_code = send(other_users[i], message, BUF_SIZE, 0); 
		if (send_code == -1) send_error = 1;
	}
	while (1) {
		free(other_users);
		recv_code = recv(th_data->s_connection_desc, text_buf, BUF_SIZE, 0);
		if (recv_code > 0) {
			strcpy(message, header);
			strcat(message, text_buf);
			other_users = getOtherUsersInRoom(rooms_list, room_number, th_data->s_connection_desc, &other_users_len);
			for (i = 0; i < other_users_len; i++) { 
				send_code = send(other_users[i], message, BUF_SIZE, 0); 
				if (send_code == -1) send_error = 1;
			}
		}
		else if (!strcmp(text_buf, ";;exit") || recv_code == 0) { 
			other_users = getOtherUsersInRoom(rooms_list, room_number, th_data->s_connection_desc, &other_users_len);
			strcpy(message, ";;");
			strcat(message, header);
			strcat(message, "leave");
			for (i = 0; i < other_users_len; i++) { 
				send_code = send(other_users[i], message, BUF_SIZE, 0); 
				if (send_code == -1) send_error = 1;
			}
			break;
		}
		else if (recv_code == -1) {
			recv_error = 1;
		}
	}
	free(other_users);
	removeUserFromRoom(&rooms_list, room_number, th_data->s_connection_desc);
clientExitRoutine:
	close(th_data->s_connection_desc);
	free(th_data);
	if (recv_code == -1) perror("Error: ");
	pthread_exit(NULL);
}

void handleConnection(int connection_desc) {
	pthread_t client_thread;
	struct client_main_thread_data *t_data;
	t_data = malloc(sizeof(struct client_main_thread_data));
	t_data->s_connection_desc = connection_desc;
	pthread_create(&client_thread, NULL, clientThread, (void *)t_data);
}

int main(int argc, char* argv[])
{
	pthread_t thread_main;
	int server_socket_descriptor, connection_desc, error_code;
	char reuse_addr_val = 1;

	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(struct sockaddr));
	//printf("type IP address for server: ");
	//scanf("%s", ip_addr);
	//printf("type port for server: ");
	//scanf("%d", &port_nr);
	server_address.sin_family = AF_INET;
	inet_pton(AF_INET, ip_addr, &(server_address.sin_addr));
	server_address.sin_port = htons(port_nr);

	server_socket_descriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_socket_descriptor == -1) goto tcpServerError;
	setsockopt(server_socket_descriptor, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse_addr_val, sizeof(reuse_addr_val));
	error_code = bind(server_socket_descriptor, (struct sockaddr*)&server_address, sizeof(server_address));
	if (error_code == -1) goto tcpServerError;
	error_code = listen(server_socket_descriptor, QUEUE_SIZE);
	if (error_code == -1) goto tcpServerError;
	//pthread_create(&thread_main, NULL, printUsersInRoomsThread, NULL);
	while (1) {
		connection_desc = accept(server_socket_descriptor, NULL, NULL);
		if (connection_desc == -1) { 
			accept_error = 1; 
			close(connection_desc);
			continue;
		}
		handleConnection(connection_desc);
	}
	close(server_socket_descriptor);
	return 0;
tcpServerError:
	perror("Error: ");
	if (server_socket_descriptor != -1) {
		close(server_socket_descriptor);
	}
	return -1;
}
