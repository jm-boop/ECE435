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
	int n;
	int time_to_exit=0;
	unsigned int serverlen;

	port=DEFAULT_PORT;

	/* Open a socket file descriptor */
	/* AF_INET means an IP network socket, not a local (AF_UNIX) one */
	/* There are other types you can open too */
	/* SOCK_DGRAM means UDP */
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
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

	/****************************************/
	/* Main client loop 			*/
	/****************************************/

	while(1) {

		/* Prompt for a message */
		printf("Please enter a message to send: ");
		memset(buffer,0,BUFFER_SIZE);

		/* Read message */
		fgets(buffer,BUFFER_SIZE-1,stdin);

		if (!strncmp(buffer,"bye",3)) time_to_exit=1;

		/* Write to socket using the "sendto" system call */
		/* You are supposed to do the ugly cast */
		serverlen=sizeof(server_addr);
		n = sendto(socket_fd,buffer,strlen(buffer),0,
			(struct sockaddr *)&server_addr,serverlen);
		if (n<0) {
			fprintf(stderr,"Error writing socket! %s\n",
				strerror(errno));
		}


		struct timeval timeout;
		fd_set fdlist,readfd;

		timeout.tv_sec = 5;	/* set 5s timeout */
		timeout.tv_usec = 0;

		FD_ZERO(&fdlist);
		FD_SET(socket_fd, &fdlist);	/* add socket fd to list */

		memcpy(&readfd, &fdlist, sizeof(fd_set));

		if (select(socket_fd+1, &readfd, NULL, NULL, &timeout) < 0) {
			fprintf(stderr,"Error on select: %s\n",strerror(errno));
			exit(1);
		}

		/* see if some data came in */
		if (FD_ISSET(socket_fd, &readfd)) {

			/* Clear buffer and read the response from the server */
			memset(buffer,0,BUFFER_SIZE);
			n = recvfrom(socket_fd, buffer, BUFFER_SIZE-1, 0,
				(struct sockaddr *)&server_addr, &serverlen);
			if (n<0) {
				fprintf(stderr,"Error reading socket! %s\n",
					strerror(errno));
			}
			/* Print the response we got */
			printf("Received back from server: %s\n\n",buffer);
		}

		/* Otherwise we timed out */
		else {
			printf("Gave up on waiting for reply from server after 5s\n");
		}

		if (time_to_exit) break;

	}

	/* All finished, close the socket/file descriptor */
	close(socket_fd);

	return 0;
}
