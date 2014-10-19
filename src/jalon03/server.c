#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>
#include "list.h"

#define LENGTH 512
#define MAX_CLIENT 2

struct c_list{
    struct list* l;
    pthread_mutex_t mutex;
    int client_nb;
    int server_sock;
    int last_sock;
};

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void list_print(struct list* l){
    
    struct list* tmp=l;
    while(tmp != NULL)
    {
        printf("Sock : %d, Username : %s\n",tmp->sock,tmp->username);
        tmp = tmp->next;
    }
    
}

char* list_concat_username(struct list* l){
    struct list* tmp=l;
    char * concat=malloc(LENGTH);
    memset(concat, 0, LENGTH);
    while(tmp != NULL)
    {
        strcat(concat,tmp->username);
        strcat(concat,"\n");
        tmp = tmp->next;
    }
    return concat;
}

char* whois_concat(struct list* l,char* buffer){
    struct list* tmp;
    char * concat=malloc(LENGTH);
    memset(concat, 0, LENGTH);
    tmp=list_find_by_username(l,buffer);

    if(tmp != NULL){
        strcat(concat,"Pseudo : ");
        strcat(concat,buffer);
        strcat(concat,"\nLogin time : ");
        strcat(concat,ctime(&tmp->time));
        strcat(concat,"IP : ");
        strcat(concat,(char *)inet_ntoa(list_addr(tmp).sin_addr));
        strcat(concat,"\nPort : ");
        char str[10];
        sprintf(str,"%d",list_addr(tmp).sin_port);
        strcat(concat,str);
        strcat(concat,"\n");
    }
    return concat;
}

int do_socket()
{
    int yes=1;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if ( fd == -1 )
        error("Socket error");
    if ( setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))==-1)
        error("Error setting socket options");
    return fd;
}

void init_serv_addr(struct sockaddr_in* server_addr, int port)
{
    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_port = htons(port);
    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = INADDR_ANY;
}

void do_bind(int sock, struct sockaddr_in addr)
{
    if ( bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1 )
        error("Bind error");
}

int do_accept(int sock, struct sockaddr* client_addr, socklen_t* client_size)
{
    int fd;
    fd = accept(sock,client_addr,client_size);
    if ( fd == -1 )
        error("Accept error");
    return fd;
}

void do_read(int client_sock, char * buffer)
{
    read(client_sock,buffer,LENGTH);
}

void do_write(int client_sock, char * buffer)
{
    write(client_sock,buffer,LENGTH);
}

void broadcast(struct list* l, char* msg, int size){
    struct list* tmp=l;
    while(tmp != NULL){
        write(tmp->sock, msg, size);
        tmp = tmp->next;
    }
}

void *client_management(void *arg){
    
    struct c_list* client=arg;
    
    int c_sock=client->last_sock;

    char buffer[LENGTH];
    memset(buffer, 0, LENGTH);
  
    pthread_mutex_unlock( &(client->mutex) ); 
    do_read(c_sock, buffer); 
    pthread_mutex_lock( &(client->mutex) );
  
    if(list_find_by_username(client->l,buffer) != NULL){
        strcat(buffer,"(1)");
        strcpy( list_username(list_find_by_sock(client->l,c_sock)),buffer);
        strcat(buffer," is your username.\n");
        do_write(c_sock,buffer);
    } else {
        strcpy( list_username(list_find_by_sock(client->l,c_sock)),buffer);
        strcat(buffer," is your username.\n");
        do_write(c_sock,buffer);
    }
    memset(buffer, 0, LENGTH);
 
    pthread_mutex_unlock( &(client->mutex) );
 
    while(strncmp(buffer, "/quit", 5))
    {
        memset(buffer, 0, LENGTH);
        
        //read what the client has to say
        do_read(c_sock, buffer);

        if(!strncmp(buffer,"/who\n",6)){
            pthread_mutex_lock( &(client->mutex) );
            strcpy(buffer,list_concat_username(client->l));
            pthread_mutex_unlock( &(client->mutex) );
        }
        if(!strncmp(buffer,"/whois ",7)){
            strcpy(buffer,&buffer[7]);
            buffer[strlen(buffer)-1]='\0';
            pthread_mutex_lock( &(client->mutex) );
            strcpy(buffer,whois_concat(client->l,buffer));
            pthread_mutex_unlock( &(client->mutex) );
        }
        if(!strncmp(buffer,"/nick ",6)){
            strcpy(buffer,&buffer[6]);
            buffer[strlen(buffer)-1]='\0';
            pthread_mutex_lock( &(client->mutex) );
            if(list_find_by_username(client->l,buffer) == NULL){
                strcpy( list_username(list_find_by_sock(client->l,c_sock)),buffer);
                strcat(buffer," is your new username.\n");
            }else{
                strcat(buffer," already exists.\n");
            }
            
            pthread_mutex_unlock( &(client->mutex) );
        }
        //we write back to the client
        do_write(c_sock, buffer);

    }
    pthread_mutex_lock( &(client->mutex) );
    client->client_nb=((client->client_nb)-1);
    
    client->l=list_find_delete(client->l,c_sock);
    //clean up client socket
    close(c_sock);
    pthread_mutex_unlock( &(client->mutex) );

    pthread_exit(NULL);
}

void *client_chat(void *arg){
    
    struct c_list* s=arg;
    struct sockaddr_in client_addr;
    int client_size;
    int client_sock;
    pthread_t pthread;
    int thr;
    time_t mytime;

    client_size = sizeof(client_addr);

    while(1){

        //accept connection from client
        client_sock = do_accept(s->server_sock, (struct sockaddr *)&client_addr, &client_size);
        if(s->client_nb < MAX_CLIENT){
            s->client_nb=((s->client_nb)+1);
            do_write(client_sock, "");
            s->last_sock=client_sock;
            mytime=time(NULL);
            s->l = list_insert_tail(s->l, client_sock, "Undefined",client_addr,mytime);
            pthread_mutex_lock( &(s->mutex) );
            thr=pthread_create(&pthread,NULL,client_management,(void*) s); 
        }else{
            do_write(client_sock, "Full server, try again later\n");
            close(client_sock);
        }

    }

    pthread_exit(NULL);
}

int main(int argc, char** argv)
{
    int sock;
    struct sockaddr_in server_addr;
    pthread_t pthread;
    int thr;
    char option[LENGTH];

    if (argc != 2)
    {
        fprintf(stderr, "usage: RE216_SERVER port\n");
        return 1;
    }

    //create the socket, check for validity!
    sock = do_socket();

    //init the serv_add structure
    init_serv_addr(&server_addr, atoi(argv[1]));

    //perform the binding
    //we bind on the tcp port specified
    do_bind(sock,server_addr);
 
    //specify the socket to be a server socket and listen for at most 20 concurrent client
    listen(sock, MAX_CLIENT);

    printf("\nServer : OK, Port : %s\n",argv[1]);

    struct c_list* s=malloc(sizeof(struct c_list));
    pthread_mutex_init( &(s->mutex), NULL );
    s->client_nb=0;
    s->server_sock=sock;
    s->l=list_new();

    thr=pthread_create(&pthread,NULL,client_chat,(void*) s);

    while(1){

        memset(option, 0, LENGTH);
        printf("-> ");
        fgets(option, LENGTH, stdin);
        if(!strncmp(option,"/who\n",6)){
            pthread_mutex_lock( &(s->mutex) );
            list_print(s->l);
            pthread_mutex_unlock( &(s->mutex) );
        }
        if(!strncmp(option,"/quit\n",7)){
            broadcast(s->l,"Server stopped working",22);
            break;
        }
        if(!strncmp(option,"/whois ",7)){
            strcpy(option,&option[7]);
            option[strlen(option)-1]='\0';
            if(list_find_by_username(s->l,option) != NULL)
                printf("Pseudo : %s\nLogin time : %sIP : %s\nPort :%d\n",option,ctime(&list_find_by_username(s->l,option)->time),(char *)inet_ntoa(list_addr(list_find_by_username(s->l,option)).sin_addr),list_addr(list_find_by_username(s->l,option)).sin_port);
        }
    }

    //clean up server socket
    close(sock);

    s->l=list_delete(s->l);

    return 0;
}
;