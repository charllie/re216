/////////////////////////////////////////////////////////////////////////////////////////
//BORDEAUX INP ENSEIRB-MATMECA
//DEPARTEMENT TELECOM
//RE216 PROGRAMMATION RESEAUX
//{daniel.negru,joachim.bruneau_-_queyreix,nicolas.herbaut}@ipb.fr
////////////////////////////////////////////////////////////////////////////////////////

#include <resolv.h>
#include <string.h>
#include <sys/select.h>
#include <stdlib.h>
#include "../commons/network.h"
#include "server.h"

int server_socket_fd;




void main(int argc, char** argv) {

	if (argc != 2) {
		fprintf(stderr, "plese specify local port to bind to\n");
		exit(-1);
	}

	//init a fresh socket
    //do_socket()
		
	//init server address structure
	//init_serv_addr()

	//perform the binding
	//do_bind()

	//specify the socket to be a server socket and listen for at most 20 concurrent client
	

	//init the fdset

	//in an infinite loop, listen to the stored sockets
	for (;;) {

		//use select to listen to sockets

		//implement the listening for incoming sockets

		// then implement the listening for already connected socket that write data to the server

	}

//clean up server socket

}
