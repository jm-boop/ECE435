#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>

#include <netdb.h>

/* Default port.  Must be from 1024 to 65536 for normal user */
#define DEFAULT_PORT	31337

#define BUFFER_SIZE	256

/* Default hostname */
#define DEFAULT_HOSTNAME	"localhost"

int main(int argc, char **argv) {

	int socket_fd;
	int port;
	struct hostent *server;
	struct sockaddr_in server_addr;
	char buffer[BUFFER_SIZE];
	int n, m;
	char msg[4];

	port=DEFAULT_PORT;

	/* Open a socket file descriptor */
	/* AF_INET means an IP network socket, not a local (AF_UNIX) one */
	/* There are other types you can open too */
	/* SOCK_STREAM means reliable two-way byte stream (TCP) */
	/* last argument is protocol subset.  We leave at zero */
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd<0) {
		fprintf(stderr,"Error socket: %s\n",
			strerror(errno));
	}

	printf("Connecting to server %s port %d\n",
		DEFAULT_HOSTNAME,port);

	/* Look up the server info based on its name */
	server=gethostbyname(DEFAULT_HOSTNAME);
	if (server==NULL) {
		fprintf(stderr,"ERROR!  No such host!\n");
		exit(0);
	}

	/* clear out the server_addr structure and set some fields */
	/* Set it to connect to the address and port of our server */
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family=AF_INET;
	memcpy(server->h_addr,&server_addr.sin_addr.s_addr,
		server->h_length);
	/* port should be in "network byte order" (big-endian) so convert */
	/* htons = host to network [byte order] short */
	server_addr.sin_port=htons(port);

	/* Call the connect system call to actually connect to server */
	if (connect(socket_fd,(struct sockaddr *) &server_addr,
		sizeof(server_addr)) < 0) {
		fprintf(stderr,"Error connecting! %s\n",
			strerror(errno));
	}


	/****************************************/
	/* Main client loop 			*/
	/****************************************/
	while (1) {
		/* Prompt for a message */
		printf("Please enter a message to send: ");
		memset(buffer,0,BUFFER_SIZE);

		/* Read message and error check */
		if (fgets(buffer,BUFFER_SIZE-1,stdin) == NULL) {
			fprintf(stderr,"Error reading input! %s\n",
				strerror(errno));
		}

		/* Write to socket using the "write" system call */
		n = write(socket_fd,buffer,strlen(buffer));
		if (n<0) {
			fprintf(stderr,"Error writing socket! %s\n",
				strerror(errno));
		}

		/* Clear buffer and read the response from the server */
		memset(buffer,0,BUFFER_SIZE);
		n = read(socket_fd,buffer,BUFFER_SIZE-1);
		if (n<0) {
			fprintf(stderr,"Error reading socket! %s\n",
				strerror(errno));
		}

		/* Print the response we got */
		printf("Received back from server: %s\n\n",buffer);
		
		/* Close socket if 'BYE' is echoed back from server */
		strcpy(msg,"BYE");
		m = strncmp(buffer,msg,3);
		if (m==0) {
			printf("Closing client\n");
			/* Close the socket */
			close(socket_fd);
			return 0;
		}
	}

	/* All finished, close the socket/file descriptor */
	close(socket_fd);

	return 0;
}
