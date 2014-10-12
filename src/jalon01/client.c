#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

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
	send(sock,msg,512,0);
}

int main(int argc,char** argv)
{
	int sock;
	struct sockaddr_in sock_host;
	char msg[512];
	char buffer[512];

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


	//get user input
	//readline()
	memset(buffer, 0, 512);
	memset(msg, 0, 512);
	printf("\nChat Room :\n\n");

	while( strncmp(msg, "/quit", 5) )
	{
		memset(buffer, 0, 512);
		memset(msg, 0, 512);
		printf("-> ");
		fgets(msg, 512, stdin);

		//send message to the server
		//handle_client_message()
		handle_client_message(sock, msg);
	
		recv(sock, buffer, 512, 0);
		printf("%s",buffer);
	}

	close(sock);

    return 0;
}
