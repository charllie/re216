#ifndef SERVER_H_
#define SERVER_H_

typedef enum arguments {
	NOTHING = 0,
	QUIT, //1
	NICK, //2
	LOGIN, //3
	WHO, //4
	WHOIS, //5
	MSGALL, //6
	MSG
} argument_t;

#endif /* SERVER_H_ */