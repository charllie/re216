#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define LENGTH 512
#define MAX_CLIENT 20

struct c_info{
    int sock;
    int client_nb;
};

void error(const char *msg)
{
    perror(msg);
    exit(1);
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

void *client_management(void *arg){
    
    struct c_info* client=arg;
    int c_sock = client->sock;
    char buffer[LENGTH];
    while(strncmp(buffer, "/quit", 5))
    {
        memset(buffer, 0, LENGTH);
        
        //read what the client has to say
        do_read(c_sock, buffer);

        //we write back to the client
        do_write(c_sock, buffer);

    }

    client->client_nb=((client->client_nb)-1);
    printf("Client exit\n");
    
    //clean up client socket
    close(c_sock);

    pthread_exit(NULL);
}

int main(int argc, char** argv)
{
    int sock, client_sock;
    int client_size;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    pthread_t pthread;
    int thr;

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

    client_size = sizeof(client_addr);

    printf("Server : OK, Port : %s\n",argv[1]);

    struct c_info* s=malloc(sizeof(struct c_info));
    s->client_nb=0;
    while(1){
        //accept connection from client
        s->sock = do_accept(sock, (struct sockaddr *)&client_addr, &client_size);
        if(s->client_nb < MAX_CLIENT){
            s->client_nb=((s->client_nb)+1);
            do_write(s->sock, "");
            thr=pthread_create(&pthread,NULL,client_management,(void*) s); 
        }else{
            do_write(s->sock, "Full server, try again later\n");
            close(s->sock);
        }

    }
    
    //clean up server socket
    close(sock);

    return 0;
}
