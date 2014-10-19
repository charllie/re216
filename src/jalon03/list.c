#include "list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

struct list* list_new()
{
	return NULL;
}

char* list_username(struct list* l)
{
	return l->username;
}

int list_sock(struct list* l)
{
	return l->sock;
}

struct sockaddr_in list_addr(struct list* l)
{
	return l->addr;
}

struct list* list_insert_head(struct list* l, int sock, char username[PLENGTH],struct sockaddr_in addr,time_t time)
{
	struct list* tmp = malloc( sizeof(*tmp) );
	strcpy(tmp->username,username);
	tmp->time=time;
	tmp->addr=addr;
	tmp->sock = sock;
	tmp->next = l;
    return tmp;
}

struct list* list_insert_tail(struct list* l, int sock, char username[PLENGTH],struct sockaddr_in addr,time_t time)
{
	struct list* tmp = malloc( sizeof(*tmp) );
	strcpy(tmp->username,username);
	tmp->time=time;
	tmp->addr=addr;
	tmp->sock = sock;
	tmp->next = NULL;

	if(l == NULL)
		return tmp;
	else{
		struct list* new = l;
		while(new->next != NULL){
			new = new->next;
		}
		new->next = tmp;
		return l;
	}
}


struct list* list_find_by_sock(struct list* l, int sock)
{
	struct list* tmp = l;
	while(tmp != NULL)
	{
		if( tmp->sock == sock )
			return tmp;
		tmp = tmp->next;
	}
	return NULL;
}

struct list* list_find_by_username(struct list* l, char username[PLENGTH])
{
	struct list* tmp = l;
	while(tmp != NULL)
	{
		if( !strcmp(tmp->username,username) )
			return tmp;
		tmp = tmp->next;
	}
	return NULL;
}

struct list* list_find_delete(struct list* l, int sock)
{
	if(l == NULL)
	        return NULL;
	if( l->sock == sock ){
	        struct list* tmp = l->next;
	        free(l);
	        tmp = list_find_delete(tmp, sock);
	        return tmp;
	} else {
		l->next = list_find_delete(l->next, sock);
		return l;
	}
}

struct list* list_delete_head(struct list* l)
{
	if(l != NULL){
		struct list* tmp = l->next;
		free(l);
		return tmp;
	}
	else {
		return NULL;
	}
}

struct list* list_delete_tail(struct list* l)
{
	if(l == NULL)
		return NULL;
	if(l->next == NULL){
		free(l);
		return NULL;
	}
	struct list* tmp = l;
	struct list* ptmp = l;
	while(tmp->next != NULL){
			ptmp = tmp;
			tmp = tmp->next;
	}
	ptmp->next = NULL;
	free(tmp);
	return l;
}

struct list* list_delete(struct list* l)
{
	if(l == NULL){
		return NULL;
	} else {
		struct list* tmp;
		tmp = l->next;
		free(l);
		return list_delete(tmp);
	}
}