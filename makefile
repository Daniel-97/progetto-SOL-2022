
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

bin/server: src/server/server.c src/server/includes/*.c
		$(CC) $(CFLAGS) src/server/server.c src/server/includes/*.c -o bin/server $(LIBS)

#### CLIENT ####

#D_POSIX_C_SOURCE server per timespec, -lm per linkare math.h
bin/client: src/client/client.c src/client/includes/*.c
		$(CC) $(CFLAGS) src/client/client.c	src/client/includes/*.c -o bin/client -D_POSIX_C_SOURCE=199309L -lm

all: $(TARGETS)

clean:
	rm -rf bin/*
	rm -rf build/server/*
	rm -rf build/client/*

test_server:
	./bin/server

test_client:
	./bin/client