#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define LENGTH 512

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

void get_addr_info(struct sockaddr_in* sock_host, int port, char *name)
{
	memset(sock_host,'\0', sizeof(*sock_host));
	sock_host->sin_family = AF_INET;
	sock_host->sin_port = htons(port);
	struct hostent *servername;
	servername = gethostbyname(name);
	inet_aton(servername, sock_host->sin_addr);
}

void do_connect(int sock, struct sockaddr_in addr)
{
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        error("Connect error");
}

void handle_client_message(int sock, char *msg)
{
	send(sock,msg,strlen(msg),0);
}

int main(int argc,char** argv)
{
	int sock;
	struct sockaddr_in sock_host;
	char msg[LENGTH];
	char buffer[LENGTH];

    if (argc != 3)
    {
        fprintf(stderr,"usage: RE216_CLIENT hostname port\n");
        return 1;
    }

	//get address info from the server
	get_addr_info(&sock_host, atoi(argv[2]), argv[1]);

	//get the socket
	sock = do_socket();

	//connect to remote socket
	do_connect(sock,sock_host);

	recv(sock, buffer, LENGTH, 0);
	if(!strncmp(buffer,"Full server, try again later",28)){
		printf("%s",buffer);
		close(sock);
		return 0;
	}

	//get user input
	//readline()
	printf("\nChat Room :\n\n");

	while( strncmp(msg, "/quit", 5) )
	{
		memset(buffer, 0, LENGTH);
		memset(msg, 0, LENGTH);
		printf("-> ");
		fgets(msg, LENGTH, stdin);

		//send message to the server
		//handle_client_message()
		handle_client_message(sock, msg);
	
		recv(sock, buffer, LENGTH, 0);
		printf("%s",buffer);
	}

	close(sock);

    return 0;
}
