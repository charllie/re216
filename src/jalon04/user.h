#ifndef USER_H_
#define USER_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "server.h"

#define LENGTH 512
#define MAX_CLIENT 20
#define PLENGTH LENGTH/MAX_CLIENT

typedef struct c_lists c_lists;
typedef struct c_lists_elt c_lists_elt;
typedef struct c_list c_list;
typedef struct c_list_elt c_list_elt;
typedef struct user user;

typedef enum flags {
	KEEP = 0,
	DESTROY,
	ACTIVE,
	PASSIVE
} flag_t;

//LISTS
c_lists * lists_new(int sock);
int lists_get_sock(c_lists * r);
void lists_delete(c_lists * r);

//ELEMENT OF LISTS
c_lists_elt * lists_get_head(c_lists * r);
c_lists_elt * lists_get_tail(c_lists * r);
c_lists_elt * next_lists_elt(c_lists_elt * m);
c_list * lists_elt_get_list(c_lists_elt * m);
c_lists_elt * lists_elt_find(c_lists * r, c_list * l);

//LIST
c_list * list_new(char name[20]);
int list_count(c_list * l);
void list_print(c_list * l);
char * list_get_name(c_list * l);
c_list * list_find_by_name(c_lists * r, char * name);
void list_delete_in_lists(c_lists * r, c_list * l);
void list_delete(c_list * l);
void list_insert(c_lists * r, c_list * l);

//ELEMENT OF A LIST
c_list_elt * list_get_head(c_list * l);
c_list_elt * list_get_tail(c_list * l);
c_list_elt * next_list_elt(c_list_elt * p);
c_list_elt* list_get_elt(c_list * l, user * u);

//USER
user * list_elt_get_user(c_list_elt * p);
int user_get_sock(user * user);
char * user_get_nickname(user * user);
user * user_new(int sock, time_t now, struct sockaddr_in addr);
void user_set_nickname(c_list * l, user * user, char * string);
user * user_find_by_nickname(c_list * l, char * nickname);
user * user_find_by_sock(c_list * l, int sock);
void list_insert_user(c_list * l, user * u);
int list_delete_user(c_list * l, user * u, flag_t flag);
int user_is_logged(user * user);
c_lists * user_get_channels(user * user);
struct sockaddr_in user_addr(user * user);
char * user_time(user * user);
void who(c_list * l, int sock);
void whois(c_list * l, user * seeker, char * nickname);

#endif /* USER_H_ */