CC = gcc
CFLAGS = -Wall -Wextra -std=c99

all: client server

client: client.c
	$(CC) $(CFLAGS) -o client client.c cypher.c

server: server.c
	$(CC) $(CFLAGS) -o server linkedlist.c server.c cypher.c

clean:
	rm -f client server *.o
