#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>

#define LENGTH 512

struct c_info{
	int sock;
	char msg[LENGTH];
	char buffer[LENGTH];
};

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int do_socket()
{
	int yes = 1;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if ( fd == -1 )
        error("Socket error");
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
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

void * msg_reception(void * arg)
{	
	struct c_info * info = arg;

	while(1){
		recv(info->sock, info->buffer, LENGTH, 0);
		printf("%s\n", info->buffer);
		if(!strcmp(info->buffer, "Server stopped working"))
			break;
	}

	pthread_exit(NULL);
}

int main(int argc, char** argv)
{
	struct c_info * info=malloc(sizeof(struct c_info));
	struct sockaddr_in sock_host;
	pthread_t pthread;
	int thr;

    if (argc != 3)
    {
        fprintf(stderr,"usage: RE216_CLIENT hostname port\n");
        return 1;
    }

	//get address info from the server
	get_addr_info(&sock_host, atoi(argv[2]), argv[1]);

	//get the socket
	info->sock = do_socket();

	//connect to remote socket
	do_connect(info->sock,sock_host);

	recv(info->sock, info->buffer, LENGTH, 0);

	//too much users
	if(!strcmp(info->buffer, "Server cannot accept incoming connections anymore. Try again later.\n")) {
		printf("%s",info->buffer);
		close(info->sock);
		return 0;
	}

	printf("Connecting to server ... done!\n");

	//pick a nickname
	printf("\033[1m[Server] : \033[0m%s\n", info->buffer);

	thr = pthread_create(&pthread, NULL, msg_reception, (void *) info);

	while(1)
	{
		memset(info->buffer, 0, LENGTH);
		memset(info->msg, 0, LENGTH);
		fgets(info->msg, LENGTH, stdin);

		//send message to the server
		handle_client_message(info->sock, info->msg);
		
		if(strcmp(info->msg, "/quit\n")) {		
			if (!strcmp(info->buffer, "Server stopped working"))
				break;
		} else {
			break;
		}
	}

	close(info->sock);

    return 0;
}
