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
    c_list_elt * p = list_get_head(l);

    while(p != NULL)
    {
    	user * u = list_elt_get_user(p);
        do_write(user_get_sock(u), msg);
        p = next_list_elt(p);
    }
}

void msgall(c_list * l, char msg[LENGTH], int sock)
{
    c_list_elt * p = list_get_head(l);
    char tmp[LENGTH];
    memset(tmp, 0, LENGTH);
    strcat(tmp, "\t\033[35m[");
    strcat(tmp, list_get_name(l));
    strcat(tmp, "]");
    strcat(tmp, "\033[36m[");
    strcat(tmp, user_get_nickname(user_find_by_sock(l, sock)));
    strcat(tmp, "] : \033[0m");
    strcat(tmp, msg);
    tmp[strlen(tmp)-1]='\0';

    while(p != NULL)
    {
    	user * u = list_elt_get_user(p);
    	if(user_get_sock(u)!= sock && user_is_logged(u))
        	do_write(user_get_sock(u), tmp);
        p = next_list_elt(p);
    }
}

void msguni(c_list * l, char msg[LENGTH], int sock)
{
    char tmp[LENGTH];
    char * token;
    int c_sock;

    memset(tmp, 0, LENGTH);
    strcpy(msg, &msg[5]);
    token = strtok(msg, " ");
    if( token != NULL ){
	    strcpy(tmp,token);
	    user * u = user_find_by_nickname(l, tmp);

	    if (u != NULL) {
	    	strcpy(msg, &msg[1 + strlen(tmp)]);
	    	memset(tmp, 0, LENGTH);
	    	strcat(tmp, "\t\033[36m[");
	    	strcat(tmp, user_get_nickname(user_find_by_sock(l, sock)));
	    	strcat(tmp, "] : \033[0m");
	    	strcat(tmp, msg);
	    	tmp[strlen(tmp)-1] = '\0';
			do_write(user_get_sock(u), tmp);
		} else {
			do_write(sock, "\033[0m[Server] : \033[1mUser not found");
		}
	}
}

void create_channel(c_lists * r, int sock, char * name) {

	c_list * l = NULL;
	char msg[LENGTH];
	memset(msg, 0, LENGTH);

	if (name != NULL) {

		if (strlen(name) > 20) {
			do_write(sock, "\033[1m[Server] : \033[0mThe name of your channel is too long");
		} else {
			l = list_find_by_name(r, name);
			if (l != NULL) {
				do_write(sock, "\033[1m[Server] : \033[0mThis channel is already created");
			} else {
				l = list_new(name);
				list_insert(r, l);
				strcat(msg, "\033[1m[Server] : \033[0mYou have created channel ");
				strcat(msg, name);
				do_write(sock, msg);
			}
		}
	} else {
		do_write(sock, "\033[1m[Server] : \033[0mPlease specify a valid name for your channel");
	} 
}

void join_channel(c_lists * index, user * u, char * channel_name) {
	int sock = user_get_sock(u);
	c_list * l = NULL;
	c_lists * user_channels = user_get_channels(u);
	char msg[LENGTH];
	memset(msg, 0, LENGTH);

	if (channel_name != NULL) {
		l = list_find_by_name(index, channel_name);
		if (l) {
			if (l != list_find_by_name(user_channels, channel_name)) {	
				list_insert_user(l, u);
				list_insert(user_channels, l);
			} else {
				//top of the user lists
				list_delete_in_lists(user_channels, l);
				list_insert(user_channels, l);
			}

			strcat(msg, "\033[1m[Server] : \033[0mYou have joined ");
			strcat(msg, channel_name);
			do_write(sock, msg);
		} else {
			do_write(sock, "\033[1m[Server] : \033[0mChannel not found");
		}
	} else {
		do_write(sock, "\033[1m[Server] : \033[0mInvalid channel name");
	}
}

void quit_channel(c_lists * index, user * u, char * channel_name) {
	int sock = user_get_sock(u);
	c_list * main_list = lists_elt_get_list(lists_get_tail(index));
	c_lists * user_channels = user_get_channels(u);
	c_list * l = NULL;
	char msg[LENGTH];
	memset(msg, 0, LENGTH);
	int users;

	if (channel_name != NULL && strcmp(channel_name, "Main")) {
		l = list_find_by_name(user_channels, channel_name);
		if (l) {
			users = list_delete_user(l, u, KEEP);
			strcat(msg, "\033[1m[Server] : \033[0mYou have left ");
			strcat(msg, channel_name);
			if (users == 0) {
				list_delete_in_lists(index, l);
				list_delete_in_lists(user_channels, l);
				list_delete(l);
				strcat(msg, " which has been destroyed");
			} else {
				list_delete_in_lists(user_channels, l);
			}
			list_delete_in_lists(user_channels, main_list);
			list_insert(user_channels, main_list);
			do_write(sock, msg);
		} else {
			do_write(sock, "\033[1m[Server] : \033[0mYou are not in this channel");
		}
	} else {
		do_write(sock, "\033[1m[Server] : \033[0mInvalid channel name");
	}
}

void quit_all_channels(c_lists * index, user * u) {
	c_lists * user_channels = user_get_channels(u);
	c_lists_elt * m = lists_get_head(user_channels);
	c_list * main_list = lists_elt_get_list(lists_get_tail(index));
	c_list * l = NULL;
	int users;

	while(m) {
		l = lists_elt_get_list(m);
		if (l == main_list){
			m = next_lists_elt(m);
			continue;
		}
		users = list_delete_user(l, u, KEEP);
		if (users == 0) {
			list_delete_in_lists(index, l);
			list_delete_in_lists(user_channels, l);
			list_delete(l);
		} else {
			list_delete_in_lists(user_channels, l);
		}
		m = next_lists_elt(m);
	}

	lists_delete(user_channels);
}

argument_t user_argument_detection(c_lists * index, user * u, char buffer[LENGTH], int sock)
{
	argument_t detected = NOTHING;
	c_lists_elt * m = lists_get_tail(index);
	c_list * l = lists_elt_get_list(m);
	int logged = user_is_logged(u);

	char buf[LENGTH];
	char * s;

	strncpy(buf, buffer, LENGTH);
	strtok(buf, "\n");
	s = strtok(buf, " ");

	while (s != NULL)
	{
		if (!strcmp(s, "/nick")) {
			user_set_nickname(l, u, strtok(NULL, " "));
			detected = NICK;
		} else if (!logged) {
			do_write(user_get_sock(u), "\033[1m[Server] : \033[0mplease logon with /nick <your pseudo>");
			detected = LOGIN;
		} else if (!strcmp(s, "/who")) {
			who(l, user_get_sock(u));
			detected = WHO;
		} else if (!strcmp(s, "/whois")) {
			whois(l, u, strtok(NULL, " "));
			detected = WHOIS;
		} else if (!strcmp(s, "/msgall") && strlen(buffer)>8) {
			memset(buf, 0, LENGTH);
			strcpy(buf, &buffer[8]);
			msgall(l, buf, sock);
			detected = MSGALL;
		} else if (!strcmp(s, "/msg")) {
			msguni(l, buffer, sock);
			detected = MSG;
		} else if (!strcmp(s, "/create")) {
			create_channel(index, user_get_sock(u), strtok(NULL, " "));
			detected = CREATE;
		} else if (!strcmp(s, "/join")) {
			join_channel(index, u, strtok(NULL, " "));
			detected = JOIN;
		} else if (!strcmp(s, "/quit")) {
			quit_channel(index, u, strtok(NULL, " "));
			detected = QUIT;
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
	char * s;

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

void * user_management(void * lists)
{	
	c_lists * index = lists;
	c_lists_elt * m = lists_get_tail(index);
	c_list * main_list = lists_elt_get_list(m);
	c_list * active_list = NULL;
	c_list_elt * p = list_get_head(main_list);
	user * u = list_elt_get_user(p);

	int sock = user_get_sock(u);
	int logged = user_is_logged(u);
	char buffer[LENGTH];
	char tmp[LENGTH];

	while(1)
	{
		memset(buffer, 0, LENGTH);
		memset(tmp, 0, LENGTH);
		//read what the client has to say
		do_read(sock, buffer);

		if (!strcmp(buffer, "/quit\n")) {
			quit_all_channels(index, u);
			break;
		}

		if (user_argument_detection(index, u, buffer,sock) != NOTHING) 
			continue;
		else {
			active_list = lists_elt_get_list(lists_get_head(user_get_channels(u)));
			if (active_list == main_list) {
				strcat(tmp, "\033[1m[Server] : \033[0m");
				strcat(tmp, buffer);
				tmp[strlen(tmp)-1]='\0';
				do_write(sock, tmp);
			} else {
				msgall(active_list, buffer, sock);
			}
		}

	}

	list_delete_user(main_list, u, DESTROY);
	//clean up client socket
	close(sock);
	pthread_exit(NULL);
}

void * client_chat(void * arg)
{
	struct c_lists * index = arg;
	struct c_lists_elt * m = lists_get_tail(index);
	struct c_list * l = lists_elt_get_list(m);
	struct sockaddr_in client_addr;

	int client_size;
	int client_sock;

	pthread_t pthread;

	int thr;

	time_t now;

	client_size = sizeof(client_addr);

	while(1) {
		//accept connection from client
		client_sock = do_accept(lists_get_sock(index), (struct sockaddr *)&client_addr, &client_size);

		if (list_count(l) < MAX_CLIENT) {
			now = time(NULL);
			user * u = user_new(client_sock, now, client_addr);
			list_insert_user(l, u);
			list_insert(user_get_channels(u), l);
			do_write(client_sock, "please logon with /nick <your pseudo>");
			pthread_create(&pthread, NULL, user_management, (void*) index);
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

	c_lists * index = lists_new(sock);
	c_list * main_list = list_new("Main");
	list_insert(index, main_list);

	//init the serv_add structure
	init_serv_addr(&server_addr, atoi(argv[1]));

	//perform the binding
	//we bind on the tcp port specified
	do_bind(sock, server_addr);
 
	//specify the socket to be a server socket and listen for at most 20 concurrent client
	listen(sock, MAX_CLIENT);

	thr = pthread_create(&pthread, NULL, client_chat, (void *) index);

    while(1)
    {
        memset(option, 0, LENGTH);
        printf("-> ");
        fgets(option, LENGTH, stdin);

        argument = server_argument_detection(option);

        if (argument == WHO) {
            list_print(main_list);
        } else if (argument == WHOIS) {
            strcpy(option, &option[7]);
            option[strlen(option)-1]='\0';
            user * u = user_find_by_nickname(main_list, option);
            if(u != NULL)
                printf("Pseudo : %s\nLogin time : %sIP : %s\nPort :%d\n", option, user_time(u), inet_ntoa(user_addr(u).sin_addr),user_addr(u).sin_port);
        } else if (argument == QUIT) {
            broadcast(main_list, "Server stopped working");
            break;
        }
    }
	
	//clean up server socket
	close(sock);
	lists_delete(index);

	return 0;
}