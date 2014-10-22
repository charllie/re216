#ifndef USER_H_
#define USER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LENGTH 512
#define MAX_CLIENT 20
#define PLENGTH LENGTH/MAX_CLIENT

typedef struct c_list c_list;
typedef struct user user;
c_list* list_new ();
int list_count(c_list* l);

void list_print(c_list * l);

int list_get_sock(c_list * l);
user * list_get_head(c_list* l);
user * list_get_tail(c_list* l);
user * next_user(user* p);
void list_delete(c_list* l);
void list_delete_user(c_list* l, user * p);
void list_insert_user(c_list* l, user * p);

user * user_new(int sock, time_t now, struct sockaddr_in addr);
void user_set_nickname(c_list* l, user * user, char * string);
user * user_find_by_nickname(c_list * l, char * nickname);
user * user_find_by_sock(c_list * l, int sock);

char * user_get_nickname(user * user);
int user_get_sock(user* user);
int user_is_logged(user* user);
struct sockaddr_in user_addr(user * user);
char* user_time(user * user);
void who(c_list * l, int sock);
void whois(c_list * l, user * seeker, char * nickname);

#endif /* USER_H_ */