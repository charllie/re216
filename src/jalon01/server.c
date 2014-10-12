#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int do_socket()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if ( fd == -1 )
        error("Socket error");
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
    read(client_sock,buffer,512);
}

void do_write(int client_sock, char * buffer)
{
    write(client_sock,buffer,512);
}

int main(int argc, char** argv)
{
    int sock, client_sock;
    int client_size;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    char buffer[512];

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
    listen(sock, 20);

    client_size = sizeof(client_addr);

    //accept connection from client
    client_sock = do_accept(sock, (struct sockaddr *)&client_addr, &client_size);
    
    while(strncmp(buffer, "/quit", 5))
    {
        //read what the client has to say
        do_read(client_sock, buffer);

        //we write back to the client
        do_write(client_sock, buffer);

        memset(buffer, 0, 512);
    }
    
    //clean up client socket
    close(client_sock);
    
    //clean up server socket
    close(sock);

    return 0;
}
