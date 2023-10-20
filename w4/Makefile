CC = gcc
CFLAGS = -Wall -g

SRC_DIR = src
HDR_DIR = headers

SOURCES = $(wildcard $(SRC_DIR)/*.c)
HEADERS = $(wildcard $(HDR_DIR)/*.h)
OBJECTS = $(SOURCES:.c=.o)

TARGET = main

all: run

run: $(OBJECTS) 
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)
	./$(TARGET) 

compile: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

