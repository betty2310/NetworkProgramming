CC = gcc
CFLAGS = -Wall -g --std=c11

HTML_FILE = file.html

CSV_OUTPUT = links.csv

all: run

run: main.o crawler.o
	$(CC) $(CFLAGS) main.o crawler.o -o resolver -lcurl -I/usr/include/libxml2 -lxml2

main.o: main.c crawler.h
	$(CC) $(CFLAGS) -c main.c -o main.o -lcurl -I/usr/include/libxml2 -lxml2

crawler.o: crawler.c crawler.h
	$(CC) $(CFLAGS) -c crawler.c -o crawler.o -lcurl

clean:
	rm -f resolver *.o $(HTML_FILE) $(CSV_OUTPUT)

