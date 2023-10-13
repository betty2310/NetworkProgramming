CC = gcc
CFLAGS = -Wall -g --std=c11

SRC_DIR = src
HDR_DIR = headers

SOURCES = $(wildcard $(SRC_DIR)/*.c)
HEADERS = $(wildcard $(HDR_DIR)/*.h)
OBJECTS = $(SOURCES:.c=.o)

MAIN_SOURCE = main.c
TARGET = resolver

all: run

run: $(OBJECTS) $(MAIN_SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(MAIN_SOURCE) -lcurl -I/usr/include/libxml2 -lxml2

compile: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(MAIN_SOURCE)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

