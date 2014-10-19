#ifndef LIST_H_
#define LIST_H_

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#define PLENGTH 512

struct list {
	char username[PLENGTH];
	int sock;
	struct sockaddr_in addr;
	time_t time;
	struct list* next;
};

// Create a new list
struct list* list_new();

// Return data
char* list_username(struct list* l);
int list_sock(struct list* l);
struct sockaddr_in list_addr(struct list* l);

// Insert data
struct list* list_insert_head(struct list* l, int sock, char username[PLENGTH],struct sockaddr_in addr,time_t time);
struct list* list_insert_tail(struct list* l, int sock, char username[PLENGTH],struct sockaddr_in addr,time_t time);

// Search an element in the list
struct list* list_find_by_sock(struct list* l, int sock);
struct list* list_find_by_username(struct list* l, char username[PLENGTH]);

// Search an element and delete it
struct list* list_find_delete(struct list* l, int sock);

// Delete data
struct list* list_delete_head(struct list* l);
struct list* list_delete_tail(struct list* l);

// Delete an entire list
struct list* list_delete(struct list* l);


#endif