CC = gcc
CFLAGS = -Wall

all: server client

server: md5.c server.c
	$(CC) $(CFLAGS) -o server md5.c server.c
	rm -rf ./storage

client: client.c
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm -f server client
