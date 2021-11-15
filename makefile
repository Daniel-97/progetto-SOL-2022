
CC = gcc
CFLAGS = -std=c99 -Wall
LIBS = -lpthread

SERVER_INCLUDE = src/server/includes
CLIENT_INCLUDE = src/client/includes

TARGETS = 	bin/server	\
			bin/client

.PHONY: all clean test1 test2 test3
.SUFFIXES: .c .h #Necessario per notazione %.c

#### SERVER ####

bin/server: src/server/server.c src/server/includes/config.c src/server/includes/connectionQueue.c
		$(CC) $(CFLAGS) src/server/server.c src/server/includes/*.c -o bin/server $(LIBS)

#### CLIENT ####

bin/client: src/client/client.c src/client/includes/utils.c
		$(CC) $(CFLAGS) src/client/client.c	src/client/includes/utils.c -o bin/client

all: $(TARGETS)

clean:
	rm -rf bin/*
	rm -rf build/server/*
	rm -rf build/client/*

test_server:
	./bin/server

test_client:
	./bin/client