CC = gcc
CFLAGS = -O2 -Wall
LFLAGS =

all:	client server

client:	client.o
	$(CC) $(LFLAGS) -o client client.o

client.o:	client.c
	$(CC) $(CFLAGS) -c client.c

server:	server.o
	$(CC) $(LFLAGS) -o server server.o

server.o:	server.c
	$(CC) $(CFLAGS) -c server.c

submit:
	tar -czvf hw1_submit.tar.gz README Makefile client.c server.c

clean:	
	rm -f *~ *.o server client
