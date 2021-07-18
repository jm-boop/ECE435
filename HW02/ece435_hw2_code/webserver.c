#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>    // open()
#include <time.h>     // time(), strftime()
#include <sys/stat.h> // stat()
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE	256

/* Listen on port 8080 */
#define DEFAULT_PORT    8080	

int main(int argc, char **argv) {

	int socket_fd,new_socket_fd;
	struct sockaddr_in server_addr, client_addr;
	int port=DEFAULT_PORT;
	int n;
	socklen_t client_len;
	char buffer[BUFFER_SIZE];

	int  j, m;	
	char *fext;
	char *ftype;
	int req_fd;
	int err_fd;
	char file[100];          // to hold: filename
	char prot[12];           //          protocol
	char *req;               //          request message from client 
	char reqfile[500];	 //          contents of requested or error file	
	char header[200];        //          contents of header
	time_t nowtime;
	struct tm *tinfo;
	char timef[40];
	time_t fmodtime;
	struct stat finfo;
	char ftimef[40];

	printf("Starting server on port %d\n",port);

	/* Open a socket to listen on */
	/* AF_INET means an IPv4 connection */
	/* SOCK_STREAM means reliable two-way connection (TCP) */
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd<0) {
		fprintf(stderr,"Error opening socket! %s\n",
			strerror(errno));
		exit(1);
	}

	/* Set up the server address to listen on */
	/* The memset stes the address to 0.0.0.0 which means */
	/* listen on any interface. */
	memset(&server_addr,0,sizeof(struct sockaddr_in));
	server_addr.sin_family=AF_INET;
	/* Convert the port we want to network byte order */
	server_addr.sin_port=htons(port);

	/* Bind to the port */
	if (bind(socket_fd, (struct sockaddr *) &server_addr,
		sizeof(server_addr)) <0) {
		fprintf(stderr,"Error binding! %s\n", strerror(errno));
		fprintf(stderr,"Probably in time wait, have to wait 60s if you ^C to close\n");
		exit(1);
	}

	/* Tell the server we want to listen on the port */
	/* Second argument is backlog, how many pending connections can */
	/* build up */
	listen(socket_fd,5);

wait_for_connection:

	/* Call accept to create a new file descriptor for an incoming */
	/* connection.  It takes the oldest one off the queue */
	/* We're blocking so it waits here until a connection happens */
	client_len=sizeof(client_addr);
	new_socket_fd = accept(socket_fd,
			(struct sockaddr *)&client_addr,&client_len);
	if (new_socket_fd<0) {
		fprintf(stderr,"Error accepting! %s\n", strerror(errno));
		exit(1);
	}

	while(1) {
		/* Someone connected!  Let's try to read BUFFER_SIZE-1 bytes */
		memset(buffer,0,BUFFER_SIZE);
		n = read(new_socket_fd,buffer,(BUFFER_SIZE-1));
		if (n==0) {
			fprintf(stderr,"Connection to client lost\n\n");
			break;
		}
		else if (n<0) {
			fprintf(stderr,"Error reading from socket %s\n",
				strerror(errno));
		}

		// Obtain message that has a 'GET' request
		// Example --> "GET /test.html HTTP/1.0"
		// NOTE: The entire message is stored into 'req', not exclusively the line containing 'GET'
		req = strstr(buffer, "GET");
		memset(file,0,100);
		memset(prot,0,12);
		memset(header,0,200);
		memset(reqfile,0,500);
		if (req != NULL) {
			// Grab system time (s) since Jan. 1, 1970
			time(&nowtime);
			// Conver system time to GMT (Greenwhich Mean Time)
			tinfo = gmtime(&nowtime);
			// Format time for the header
			strftime(timef, 40, "%a, %d %b %Y %X %Z", tinfo);

			j = 0;
			// Iterate over the leading '/' before filename		
			// NOTE: Won't work if trying to compare "/", since double quotations specify string ending with a '\0'
			while (*req != '/') req++;
			req++;
			// Obtain the filename, iterating through 'req' pointer
			while (*req != ' ') {
				file[j] = *req;
				req++;
				j++;
			}
			// Since assigning characters from pointer 'req', need to append a null terminator to end the string
			file[j] = '\0';
			req++;
			j = 0;
			// Obtain the protocol
			while(*req != '\n') {
				prot[j] = *req;
				req++;
				j++;
			}
			prot[j] = '\0';

			// Grab file extension
			fext = strstr(file, ".");
			// Map file extension to popular file types
			if (!strcmp(fext, ".html")) {
				ftype = "text/html";
			} else if (strcmp(fext, ".txt")) {
				ftype = "text/plain";
			} else if (strcmp(fext, ".png")) {
				ftype = "image/png";
			} else if (strcmp(fext, ".jpeg")) {
				ftype = "image/jpeg";
			}

			// Print filename requested
			printf("File requested: %s\n", file);
	
			// Once file name parsed, attempt to open it
			req_fd = open(file, O_RDONLY);
			if (req_fd < 0) {
				fprintf(stderr, "File '%s' not found. %s\n",
					file, strerror(errno));
				if ((err_fd = open("error.html", O_RDONLY)) > 0) {	
					m = read(err_fd, reqfile, 500 - 1);
					if (m == 0) {
						fprintf(stderr, "EOF reached\n\n");
					} else if (m < 0) {
						fprintf(stderr, "Error reading file contents %s\n",
						strerror(errno));
					} else {
						sprintf(header, "HTTP/1.1 404 Not Found\r\nDate: %s\r\nServer: ECE435\r\nContent-Length: %d\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n", timef, m);
						// Write header to client
						//printf("%s\n", header);
						n = write(new_socket_fd, header, strlen(header));	
						if (n<0) {
							fprintf(stderr, "Error writing. %s\n",
							strerror(errno));
						}
						// Write file contents to client
						// printf("%s\n", reqfile);
						n = write(new_socket_fd, reqfile, m);
						if (n<0) {
							fprintf(stderr, "Error writing. %s\n",
							strerror(errno));
						}
					}
				}
				
			} else {
				// Open was successfull! Attempt to read file contents into 'reqfile'	
				m = read(req_fd, reqfile, 500 - 1);
				printf("\n\nm = %d\n\n", m);
				if (m == 0) {
					fprintf(stderr,"EOF reached\n\n");
				} else if (m < 0) {
					fprintf(stderr,"Error reading file contents %s\n",
					strerror(errno));
				} else {	
					stat(file, &finfo);
					fmodtime = finfo.st_mtim.tv_sec;
					strftime(ftimef, 40, "%a, %d %b %Y %X %Z", gmtime(&fmodtime));	
					
					sprintf(header, "HTTP/1.1 200 OK\r\nDate: %s\r\nServer: ECE435\r\nLast-Modified: %s\r\nContent-Length: %d\r\nContent-Type: %s\r\n\r\n", timef, ftimef, m, ftype);
					// Write header to client
					printf("%s\n", header);
					n = write(new_socket_fd, header, strlen(header));	
					if (n<0) {
						fprintf(stderr, "Error writing. %s\n",
						strerror(errno));
					}
					// Write file contents to client
					// printf("%s\n", reqfile);
					n = write(new_socket_fd, reqfile, m);
					if (n<0) {
						fprintf(stderr, "Error writing. %s\n",
						strerror(errno));
					}
				}
				close(req_fd);
			}				
		}	

		/* Print the message we received */
		printf("Message received: %s\n",buffer);

		/* Send a response */
		n = write(new_socket_fd,"Got your message, thanks!\r\n\r\n",29);
		if (n<0) {
			fprintf(stderr,"Error writing. %s\n",
				strerror(errno));
		}

	}

	close(new_socket_fd);

	printf("Done connection, go back and wait for another\n\n");

	goto wait_for_connection;

	/* Try to avoid TIME_WAIT */
	sleep(1);

	/* Close the sockets */
	close(socket_fd);

	return 0;
}
