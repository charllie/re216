#include <stdlib.h>
#include "user.h"

struct c_list {
	int count;
	int server_sock;
	user * head;
	user * tail;
};

struct user {
	int logged;
	int sock;
	char nickname[PLENGTH];
	time_t time;
	struct sockaddr_in addr;
	user * next;
	user * prev;
};

c_list * list_new(int sock) {
	c_list * l = malloc(sizeof(*l));
	l->count = 0;
	l->server_sock = sock;
	l->head = NULL;
	l->tail = NULL;
	return l;
}

int list_get_sock(c_list * l) {
	return l->server_sock;
}

int list_count(c_list * l) {
	return l->count;
}

void list_print(c_list * l)
{
    user * p = list_get_head(l);
    while(p != NULL)
    {
        printf("Sock : %d, Username : %s\n", p->sock, p->nickname);
        p = p->next;
    }
}

user * list_get_head(c_list * l) {
	return l->head;
}

user * list_get_tail(c_list * l) {
	return l->tail;
}

user * next_user(user * p) {
	return p->next;
}

void list_delete(c_list* l) {
	user * p = list_get_head(l);
	while(p) {
		user * q = p;
		p = p->next;
		free(q);
	}
	free(l);
}

void list_delete_user(c_list * l, user * p) {
	if (p) {
		if (p->prev && p->next) {
			p->prev->next = p->next;
			p->next->prev = p->prev;
		} else if (p->prev) {
			l->tail = p->prev;
			p->prev->next = NULL;
		} else if (p->next) {
			l->head = p->next;
			p->next->prev = NULL;
		} else {
			l->head = NULL;
			l->tail = NULL;
		}
		l->count--;
		free(p);
	}
}

void list_insert_user(c_list* l, user * p) {
	if (l->head == NULL) {
		p->next = NULL;
		p->prev = NULL;
		l->head = p;
		l->tail = p;
	} else {
		p->next = l->head;
		p->prev = NULL;
		l->head->prev = p;
		l->head = p;
	}
	l->count++;
}

user * user_new(int sock, time_t now, struct sockaddr_in addr)
{
	user * user = malloc(sizeof(struct user));
	user->sock = sock;
	user->logged = 0;
	user->time = now;
	user->addr = addr;
	return user;
}

char * user_get_nickname(user * user) { return user->nickname; }

void user_set_nickname(c_list * l, user * user, char * string)
{
	if (string != NULL)
	{
		//search if another user has already this nickname
		if (user_find_by_nickname(l, string)) {
			if (!strncmp(user->nickname, string, PLENGTH)) {
				if (user->logged)
					do_write(user->sock, "[Server] : This nickname is already yours");
				else {
					strncpy(user->nickname, string, PLENGTH);

					if (strlen(string) > PLENGTH)
						user->nickname[PLENGTH] = '\0';

					char buffer[LENGTH] = "[Server] : Welcome on the chat ";
					strncat(buffer, user->nickname, LENGTH);
					do_write(user->sock, buffer);
					user->logged = 1;
				}
			} else {
				//unfortunately, this nickname has already been taken
				if (user->logged)
					do_write(user->sock, "[Server] : This nickname is already used");
				else
					do_write(user->sock, "[Server] : This nickname is already used - Please logon with /nick <your pseudo>");
			}
		//it's OK
		} else {
			strncpy(user->nickname, string, PLENGTH);

			if (strlen(string) > PLENGTH)
				user->nickname[PLENGTH] = '\0';

			if (user->logged == 1) {
				do_write(user->sock, "Nickname changed");
			} else {
				char buffer[LENGTH] = "[Server] : Welcome on the chat ";
				strncat(buffer, user->nickname, LENGTH);
				do_write(user->sock, buffer);
				user->logged = 1;
			}
		}
	} else {
		do_write(user->sock, "[Server] : Please logon with /nick <your pseudo>\n");
	}
}

user * user_find_by_nickname(c_list * l, char * nickname)
{
	user * p = list_get_head(l);

	if (strlen(nickname) > PLENGTH)
		return NULL;

	while (p != NULL)
	{
		if (!strcmp(p->nickname, nickname))
			return p;

		p = next_user(p);
	}

	return NULL;
}

user * user_find_by_sock(c_list * l, int sock)
{
	user * p = list_get_head(l);

	while (p != NULL)
	{
		if (p->sock == sock)
			return p;

		p = next_user(p);
	}

	return NULL;
}

//get the user' sock
int user_get_sock(user * user) { return user->sock; }

//is the user logged ?
int user_is_logged(user * user) { return user->logged; }

struct sockaddr_in user_addr(user * user) { return user->addr; }

char* user_time(user * user) { return ctime(&user->time); }

//display nicknames of people connected
void who(c_list * l, int sock)
{
	user * p = list_get_head(l);
	char nickname[LENGTH] = "[Server] : Online users are";

	while(p != NULL)
	{
		if (user_is_logged(p)) {
			strcat(nickname, "\n\t\t- ");
			strcat(nickname, user_get_nickname(p));		
		}

		p = next_user(p);
	}

	do_write(sock, nickname);
}

void whois(c_list * l, user * seeker, char * nickname)
{
	char buffer[LENGTH];
	char str[10];
	strcpy(buffer, "[Server] : User not found");

	if (nickname != NULL)
	{
		user * found = user_find_by_nickname(l, nickname);
		
		if (found != NULL)
		{
			memset(buffer, 0, LENGTH);
			strcat(buffer, "[Server] : ");
			strcat(buffer, nickname);
			strcat(buffer, " is connected since ");
			strcat(buffer, ctime(&found->time));
			strcat(buffer, " with IP address ");
			strcat(buffer, inet_ntoa(found->addr.sin_addr));
			strcat(buffer, " and port number ");
			sprintf(str, "%d", found->addr.sin_port);
			strcat(buffer, str);
		} 
	}

	do_write(seeker->sock, buffer);
}