
CC = gcc
CFLAGS = -std=c99 -Wall
LIBS = -pthread

TARGETS = 	server	\
			client

.PHONY: all clean test1 test2 test3

server: server.o
		$(CC) $(CFLAGS) src/server/server.c -o build/exe/server $(LIBS)
server.o: src/server/server.c
		$(CC) $(CFLAGS) -c src/server/server.c -o build/object/server/server.o

client: client.o
		$(CC) $(CFLAGS) src/client/client.c -o build/exe/client $(LIBS)
client.o: src/client/client.c
		$(CC) $(CFLAGS) -c src/client/client.c -o build/object/client/client.o

all: $(TARGETS)

clean:
	rm -rf build/object/server/*
	rm -rf build/object/client/*

test1:
	./server