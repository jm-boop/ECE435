CC = gcc
CFLAGS = -O2 -Wall
LFLAGS =

all:	webserver

webserver:	webserver.o
	$(CC) $(LFLAGS) -o webserver webserver.o

webserver.o:	webserver.c
	$(CC) $(CFLAGS) -c webserver.c

submit:
	tar -czvf hw2_submit.tar.gz README Makefile webserver.c test.html

clean:	
	rm -f *~ *.o webserver
