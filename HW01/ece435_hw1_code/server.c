#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>     // toupper()
#include <arpa/inet.h> // inet_ntop()

#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE	256

/* Default port to listen on */
#define DEFAULT_PORT	31337

int main(int argc, char **argv) {

	int socket_fd,new_socket_fd;
	struct sockaddr_in server_addr;
	struct sockaddr client_addr;
	int port=DEFAULT_PORT;
	int n, m, i;
	socklen_t client_len;
	char buffer[BUFFER_SIZE];
	char msg[4];                     // to hold 'bye'/'BYE' string
	char ip_addr[INET_ADDRSTRLEN];   // to hold client/server IPv4 address
	unsigned short addr_family;      // to hold sa_family attribute from client_addr
	int client_port;


	printf("Starting server on port %d\n",port);

	/* Open a socket to listen on */
	/* AF_INET means an IPv4 connection */
	/* SOCK_STREAM means reliable two-way connection (TCP) */
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd<0) {
		fprintf(stderr,"Error opening socket! %s\n",
			strerror(errno));
	}

	/* Set up the server address to listen on */
	/* The memset sets the address to 0.0.0.0 which means */
	/* listen on any interface. */
	memset(&server_addr,0,sizeof(struct sockaddr_in));
	server_addr.sin_family=AF_INET;
	/* Convert the port we want to network byte order */
	server_addr.sin_port=htons(port);

	/* Bind to the port */
	if (bind(socket_fd, (struct sockaddr *) &server_addr,
		sizeof(server_addr)) <0) {
		fprintf(stderr,"Error binding! %s\n", strerror(errno));
	}

	/* Tell the server we want to listen on the port */
	/* Second argument is backlog, how many pending connections can */
	/* build up */
	listen(socket_fd,5);

	/* Call accept to create a new file descriptor for an incoming */
	/* connection.  It takes the oldest one off the queue */
	/* We're blocking so it waits here until a connection happens */
	client_len=sizeof(client_addr);
	new_socket_fd = accept(socket_fd,
		(struct sockaddr *)&client_addr,&client_len);
	if (new_socket_fd<0) {
		fprintf(stderr,"Error accepting! %s\n",strerror(errno));
	}

	/*** SOMETHING COOL ***/
	addr_family = client_addr.sa_family;
	/* Convert client's IPv4 address from binary to text from */
	inet_ntop(addr_family, 
	          &((struct sockaddr_in *)&client_addr)->sin_addr.s_addr, 
                  ip_addr, INET_ADDRSTRLEN);
	/* Convert client's port from network byte order back to host byte order */
	client_port = ntohs(((struct sockaddr_in *)&client_addr)->sin_port);
	/* Once a client is connected, print port and address of client */
	printf("Client's IPv4 address: %s\n", ip_addr);
	printf("Client's Port:         %d\n", client_port);
	/**********************/

	/* 
         * Loop forever in the SAME SOCKET reading 
         * from the file descriptor and responding 
         */
	while (1) {
		/* Someone connected!  Let's try to read BUFFER_SIZE-1 bytes */
		memset(buffer,0,BUFFER_SIZE);
		n = read(new_socket_fd,buffer,(BUFFER_SIZE-1));
		if (n==0) {
			fprintf(stderr,"Connection to client lost\n\n");
		}
		else if (n<0) {
			fprintf(stderr,"Error reading from socket %s\n",
				strerror(errno));
		}

		/* Convert all lowercase characters to uppercase */
		for (i = 0; buffer[i] != '\0'; i++) {
			buffer[i] = toupper(buffer[i]);
		}
	
		/* Print the message we received */
		printf("Message from client: %s\n",buffer);

		/* Send back the message we received */
		n = write(new_socket_fd,buffer,strlen(buffer));
		if (n<0) {
			fprintf(stderr,"Error writing. %s\n",
				strerror(errno));
		}

		/* Close socket if 'BYE' is received from client */
		strcpy(msg, "BYE");
		m = strncmp(buffer, msg, 3);
		if (m==0) {
			printf("Closing server\n\n");
			/* Try to avoid TIME_WAIT */
			sleep(1);
			/* Close the sockets */
			close(new_socket_fd);
			close(socket_fd);			

			return 0;
		}
	}

	printf("Exiting server\n\n");

	/* Try to avoid TIME_WAIT */
	sleep(1);

	/* Close the sockets */
	close(new_socket_fd);
	close(socket_fd);

	

	return 0;
}
