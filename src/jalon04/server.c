#include <pthread.h>
#include "user.h"
#include "server.h"

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

int do_socket()
{
	int yes = 1;
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
		error("Socket error");
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
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
	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		error("Bind error");
}

int do_accept(int sock, struct sockaddr* client_addr, socklen_t* client_size)
{
	int fd;
	fd = accept(sock, client_addr, client_size);
	if (fd == -1)
		error("Accept error");
	return fd;
}

void do_read(int client_sock, char * buffer)
{
	read(client_sock, buffer, LENGTH);
}

void do_write(int client_sock, char * buffer)
{
	write(client_sock, buffer, LENGTH);
}

void broadcast(c_list * l, char msg[22])
{
    user * p = list_get_head(l);

    while(p != NULL)
    {
        do_write(user_get_sock(p), msg);
        p = next_user(p);
    }
}

void msgall(c_list * l, char msg[LENGTH], int sock)
{
    user * p = list_get_head(l);
    char tmp[LENGTH];
    memset(tmp, 0, LENGTH);
    strcat(tmp,"[");
    strcat(tmp,user_get_nickname(user_find_by_sock(l,sock)));
    strcpy(msg,&msg[8]);
    strcat(tmp,"] : ");
    strcat(tmp,msg);
    tmp[strlen(tmp)-1]='\0';

    while(p != NULL)
    {
    	if(user_get_sock(p)!= sock && user_is_logged(p))
        	do_write(user_get_sock(p), tmp);
        p = next_user(p);
    }
}

void msguni(c_list * l, char msg[LENGTH],int sock)
{
    char tmp[LENGTH];
    char * token;
    int c_sock;

    memset(tmp, 0, LENGTH);
    strcpy(msg,&msg[5]);
    token=strtok(msg," ");
    strcpy(tmp,token);
    user * p = user_find_by_nickname(l,tmp);

    if( p != NULL ){
    
    	strcpy(msg,&msg[1+strlen(tmp)]);
    	memset(tmp, 0, LENGTH);
    	strcat(tmp,"[");
    	strcat(tmp,user_get_nickname(user_find_by_sock(l,sock)));
    	strcat(tmp,"] : ");
    	strcat(tmp,msg);
    	tmp[strlen(tmp)-1]='\0';
		do_write(user_get_sock(p), tmp);
	} else {
		do_write(sock, "[Server] : User not found");
	}
    
}

argument_t user_argument_detection(c_list * l, user * p, char buffer[LENGTH], int sock)
{
	argument_t detected = NOTHING;
	int logged = user_is_logged(p);

	char buf[LENGTH];
	char *s;

	strncpy(buf, buffer, LENGTH);
	strtok(buf, "\n");
	s = strtok(buf, " ");

	while (s != NULL)
	{
		if (!strcmp(s, "/nick")) {
			user_set_nickname(l, p, strtok(NULL, " "));
			detected = NICK;
		} else if (!logged) {
			do_write(user_get_sock(p), "[Server] : please logon with /nick <your pseudo>");
			detected = LOGIN;
		} else if (!strcmp(s, "/who")) {
			who(l, user_get_sock(p));
			detected = WHO;
		} else if (!strcmp(s, "/whois")) {
			whois(l, p, strtok(NULL, " "));
			detected = WHOIS;
		} else if (!strcmp(s, "/msgall")) {
			msgall(l,buffer,sock );
			detected = MSGALL;
		} else if (!strcmp(s, "/msg")) {
			msguni(l,buffer,sock);
			detected = MSG;
		}

		s = strtok(NULL, " ");

		if (detected)
			break;
	}

	return detected;
}

argument_t server_argument_detection(char buffer[LENGTH])
{
	argument_t detected = NOTHING;

	char buf[LENGTH];
	char *s;

	strncpy(buf, buffer, LENGTH);
	strtok(buf, "\n");
	s = strtok(buf, " ");

	while (s != NULL)
	{
		if (!strcmp(s, "/nick")) {
			detected = NICK;
		} else if (!strcmp(s, "/who")) {
			detected = WHO;
		} else if (!strcmp(s, "/whois")) {
			detected = WHOIS;
		} else if (!strcmp(s, "/quit")) {
			detected = QUIT;
		}

		s = strtok(NULL, " ");

		if (detected)
			break;
	}

	return detected;
}

void * user_management(void * list)
{	
	c_list * l = list;
	user * p = list_get_head(l);

	int sock = user_get_sock(p);
	int logged = user_is_logged(p);
	char buffer[LENGTH];
	char tmp[LENGTH];

	while(1)
	{
		memset(buffer, 0, LENGTH);
		memset(tmp, 0, LENGTH);
		//read what the client has to say
		do_read(sock, buffer);

		if (!strcmp(buffer, "/quit\n"))
			break;

		if (user_argument_detection(l, p, buffer,sock) != NOTHING) 
			continue;
		else{
			strcat(tmp,"[Server] : ");
			strcat(tmp,buffer);
			tmp[strlen(tmp)-1]='\0';
			do_write(sock, tmp);	
		}

	}

	list_delete_user(l, p);
	//clean up client socket
	close(sock);
	pthread_exit(NULL);
}

void * client_chat(void * arg)
{
	struct c_list * l = arg;
	struct sockaddr_in client_addr;

	int client_size;
	int client_sock;

	pthread_t pthread;

	int thr;

	time_t now;

	client_size = sizeof(client_addr);

	while(1) {
		//accept connection from client
		client_sock = do_accept(list_get_sock(l), (struct sockaddr *)&client_addr, &client_size);

		if (list_count(l) < MAX_CLIENT) {
			now = time(NULL);
			user * p = user_new(client_sock, now, client_addr);
			list_insert_user(l, p);
			do_write(client_sock, "please logon with /nick <your pseudo>");
			pthread_create(&pthread, NULL, user_management, (void*) l);
		} else {
			do_write(client_sock, "Server cannot accept incoming connections anymore. Try again later.\n");
			close(client_sock);
		}
	}

	pthread_exit(NULL);
}

int main(int argc, char ** argv)
{
	int sock;
	int thr;
	argument_t argument;

	struct sockaddr_in server_addr;

	pthread_t pthread;
    char option[LENGTH];

	if (argc != 2)
	{
		fprintf(stderr, "usage: RE216_SERVER port\n");
		return 1;
	}

	//create the socket, check for validity!
	sock = do_socket();

	c_list * l = list_new(sock);

	//init the serv_add structure
	init_serv_addr(&server_addr, atoi(argv[1]));

	//perform the binding
	//we bind on the tcp port specified
	do_bind(sock, server_addr);
 
	//specify the socket to be a server socket and listen for at most 20 concurrent client
	listen(sock, MAX_CLIENT);

	thr = pthread_create(&pthread, NULL, client_chat, (void *) l);

    while(1)
    {
        memset(option, 0, LENGTH);
        printf("-> ");
        fgets(option, LENGTH, stdin);

        argument = server_argument_detection(option);

        if (argument == WHO) {
            list_print(l);
        } else if (argument == WHOIS) {
            strcpy(option,&option[7]);
            option[strlen(option)-1]='\0';
            user * p = user_find_by_nickname(l,option);
            if( p != NULL)
                printf("Pseudo : %s\nLogin time : %sIP : %s\nPort :%d\n",option,user_time(p),inet_ntoa(user_addr(p).sin_addr),user_addr(p).sin_port);
        } else if (argument == QUIT) {
            broadcast(l, "Server stopped working");
            break;
        }
    }
	
	//clean up server socket
	close(sock);
	list_delete(l);

	return 0;
}