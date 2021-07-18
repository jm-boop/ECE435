CC = gcc
CFLAGS = -g -O2 -Wall
LFLAGS =

all:	udp_client udp_server

udp_client:	udp_client.o
	$(CC) $(LFLAGS) -o udp_client udp_client.o

udp_client.o:	udp_client.c
	$(CC) $(CFLAGS) -c udp_client.c

udp_server:	udp_server.o
	$(CC) $(LFLAGS) -o udp_server udp_server.o

udp_server.o:	udp_server.c
	$(CC) $(CFLAGS) -c udp_server.c

submit:
	tar -czvf hw4_submit.tar.gz Makefile udp_client.c udp_server.c

clean:	
	rm -f *~ *.o udp_server udp_client
