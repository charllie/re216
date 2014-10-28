#include "user.h"

struct c_lists {
	c_lists_elt * head;
	c_lists_elt * tail;
	int sock;
};

struct c_lists_elt {
	c_list * list;
	c_lists_elt * prev;
	c_lists_elt * next;
};

struct c_list {
	int count;
	char name[20];
	c_list_elt * head;
	c_list_elt * tail;
};

struct c_list_elt {
	user * user;
	c_list_elt * prev;
	c_list_elt * next;
};

struct user {
	int logged;
	int sock;
	char nickname[PLENGTH];
	time_t time;
	struct sockaddr_in addr;
	c_lists * channels;
};

//LISTS
c_lists * lists_new(int sock) {
	c_lists * r = malloc(sizeof(*r));
	r->sock = sock;
	r->head = NULL;
	r->tail = NULL;
	return r;
}

int lists_get_sock(c_lists * r) {
	return r->sock;
}

void lists_delete(c_lists * r) {
	c_lists_elt * m = lists_get_head(r);
	while(m) {
		c_lists_elt * o = m;
		m = m->next;
		free(o);
	}
	free(r);
}

//ELEMENT OF LISTS
c_lists_elt * lists_get_head(c_lists * r) {
	return r->head;
}

c_lists_elt * lists_get_tail(c_lists * r) {
	return r->tail;
}

c_lists_elt * next_lists_elt(c_lists_elt * m) {
	return m->next;
}

c_list * lists_elt_get_list(c_lists_elt * m) {
	return m->list;
}

c_lists_elt * lists_elt_find(c_lists * r, c_list * l) {
	c_lists_elt * m = lists_get_head(r);
	
	while (m) {
		if (lists_elt_get_list(m) == l)
			return m;
		m = next_lists_elt(m);
	}

	return NULL;
}

//LIST
c_list * list_new(char name[20]) {
	c_list * l = malloc(sizeof(*l));
	strncpy(l->name, name, 20);
	l->count = 0;
	l->head = NULL;
	l->tail = NULL;
	return l;
}

int list_count(c_list * l) {
	return l->count;
}

void list_print(c_list * l)
{
	c_list_elt * p = list_get_head(l);
    
    while(p != NULL)
    {
    	user * u = list_elt_get_user(p);
        printf("Sock : %d, Username : %s\n", user_get_sock(u), user_get_nickname(u));
        p = p->next;
    }
}

char * list_get_name(c_list * l) {
	return l->name;
}

c_list * list_find_by_name(c_lists * r, char * name)
{
	c_lists_elt * m = lists_get_head(r);
	c_list * l = NULL;

	if (strlen(name) > 20)
		return NULL;

	while (m != NULL)
	{
		l = lists_elt_get_list(m);
		if (!strcmp(list_get_name(l), name))
			return l;

		m = next_lists_elt(m);
	}

	return NULL;
}

void list_delete_in_lists(c_lists * r, c_list * l) {

	c_lists_elt * m = lists_elt_find(r, l);

	if (m) {
		if (m->prev && m->next) {
			m->prev->next = m->next;
			m->next->prev = m->prev;
		} else if (m->prev) {
			r->tail = m->prev;
			m->prev->next = NULL;
		} else if (m->next) {
			r->head = m->next;
			m->next->prev = NULL;
		} else {
			r->head = NULL;
			r->tail = NULL;
		}
		free(m);
	}
}

void list_delete(c_list * l) {
	c_list_elt * p = list_get_head(l);
	while(p) {
		c_list_elt * q = p;
		p = p->next;
		free(q);
	}
	free(l);
}

void list_insert(c_lists * r, c_list * l) {
	c_lists_elt * m = malloc(sizeof(*m));
	m->list = l;
	if (r->head == NULL) {
		m->next = NULL;
		m->prev = NULL;
		r->head = m;
		r->tail = m;
	} else {
		m->next = r->head;
		m->prev = NULL;
		r->head->prev = m;
		r->head = m;
	}
}

//ELEMENT OF A LIST
c_list_elt * list_get_head(c_list * l) {
	return l->head;
}

c_list_elt * list_get_tail(c_list * l) {
	return l->tail;
}

c_list_elt * next_list_elt(c_list_elt * p) {
	return p->next;
}

c_list_elt* list_get_elt(c_list * l, user * u) {
	c_list_elt * p = list_get_head(l);

	while(p) {
		if(p->user == u)
			break;
		p = p->next;
	}

	return p;
}

//USER
user * list_elt_get_user(c_list_elt * p) {
	return p->user;
}

int user_get_sock(user * user) {
	return user->sock;
}

char * user_get_nickname(user * user) {
	return user->nickname;
}

user * user_new(int sock, time_t now, struct sockaddr_in addr)
{
	user * user = malloc(sizeof(struct user));
	user->sock = sock;
	user->logged = 0;
	user->time = now;
	user->addr = addr;
	user->channels = lists_new(sock);
	return user;
}

void user_set_nickname(c_list * l, user * user, char * string)
{
	if (string != NULL)
	{
		//search if another user has already this nickname
		if (user_find_by_nickname(l, string)) {
			if (!strncmp(user->nickname, string, PLENGTH)) {
				if (user->logged)
					do_write(user->sock, "\033[1m[Server] : \033[0mThis nickname is already yours");
				else {
					strncpy(user->nickname, string, PLENGTH);

					if (strlen(string) > PLENGTH)
						user->nickname[PLENGTH] = '\0';

					char buffer[LENGTH] = "\033[1m[Server] : \033[0mWelcome on the chat ";
					strncat(buffer, user->nickname, LENGTH);
					do_write(user->sock, buffer);
					user->logged = 1;
				}
			} else {
				//unfortunately, this nickname has already been taken
				if (user->logged)
					do_write(user->sock, "\033[1m[Server] : \033[0mThis nickname is already used");
				else
					do_write(user->sock, "\033[1m[Server] : \033[0mThis nickname is already used - Please logon with /nick <your pseudo>");
			}
		//it's OK
		} else {
			strncpy(user->nickname, string, PLENGTH);

			if (strlen(string) > PLENGTH)
				user->nickname[PLENGTH] = '\0';

			if (user->logged == 1) {
				do_write(user->sock, "Nickname changed");
			} else {
				char buffer[LENGTH] = "\033[1m[Server] : \033[0mWelcome on the chat ";
				strncat(buffer, user->nickname, LENGTH);
				do_write(user->sock, buffer);
				user->logged = 1;
			}
		}
	} else {
		do_write(user->sock, "\033[1m[Server] : \033[0mPlease logon with /nick <your pseudo>\n");
	}
}

user * user_find_by_nickname(c_list * l, char * nickname)
{
	c_list_elt * p = list_get_head(l);

	if (strlen(nickname) > PLENGTH)
		return NULL;

	while (p != NULL)
	{
		user * u = list_elt_get_user(p);
		if (!strcmp(u->nickname, nickname))
			return u;

		p = next_list_elt(p);
	}

	return NULL;
}

user * user_find_by_sock(c_list * l, int sock)
{
	c_list_elt * p = list_get_head(l);

	while (p != NULL)
	{
		user * u = list_elt_get_user(p);
		if (u->sock == sock)
			return u;

		p = next_list_elt(p);
	}

	return NULL;
}

void list_insert_user(c_list * l, user * u) {
	c_list_elt * p = malloc(sizeof(*p));
	p->user = u;
	if (l->head == NULL) {
		p->next = NULL;
		p->prev = NULL;
		l->head = p;
		l->tail = p;
	} else {
		p->next = l->head;
		p->prev = NULL;
		l->head->prev = p;
		l->head = p;
	}
	l->count++;
}

int list_delete_user(c_list * l, user * u, flag_t flag) {
	c_list_elt* p = list_get_elt(l, u);

	if (p) {
		if (p->prev && p->next) {
			p->prev->next = p->next;
			p->next->prev = p->prev;
		} else if (p->prev) {
			l->tail = p->prev;
			p->prev->next = NULL;
		} else if (p->next) {
			l->head = p->next;
			p->next->prev = NULL;
		} else {
			l->head = NULL;
			l->tail = NULL;
		}
		l->count--;
		if (flag == DESTROY)
			free(u);
		free(p);
	}

	return l->count;
}

int user_is_logged(user * user) {
	return user->logged;
}

c_lists * user_get_channels(user * user) {
	return user->channels;
}

struct sockaddr_in user_addr(user * user) {
	return user->addr;
}

char * user_time(user * user) {
	return ctime(&user->time);
}

void who(c_list * l, int sock)
{
	c_list_elt * p = list_get_head(l);
	char nickname[LENGTH] = "\033[1m[Server] : \033[0mOnline users are";

	while(p != NULL)
	{
		user * u = list_elt_get_user(p);
		if (user_is_logged(u)) {
			strcat(nickname, "\n\t\t- ");
			strcat(nickname, user_get_nickname(u));		
		}
		p = next_list_elt(p);
	}

	do_write(sock, nickname);
}

void whois(c_list * l, user * seeker, char * nickname)
{
	char buffer[LENGTH];
	char str[10];
	strcpy(buffer, "\033[1m[Server] : \033[0mUser not found");
	c_list_elt * p = list_get_head(l);

	if (nickname != NULL)
	{
		user * found = user_find_by_nickname(l, nickname);
		
		if (found != NULL)
		{
			memset(buffer, 0, LENGTH);
			strcat(buffer, "\033[1m[Server] : \033[0m");
			strcat(buffer, nickname);
			strcat(buffer, " is connected since ");
			strcat(buffer, ctime(&found->time));
			strcat(buffer, " with IP address ");
			strcat(buffer, inet_ntoa(found->addr.sin_addr));
			strcat(buffer, " and port number ");
			sprintf(str, "%d", found->addr.sin_port);
			strcat(buffer, str);
		} 
	}

	do_write(seeker->sock, buffer);
}