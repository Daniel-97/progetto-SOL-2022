
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

bin/server: build/server/server.o build/server/configlib.so build/server/connectionQueuelib.so
		$(CC) build/server/server.o -Wl,-rpath=${PWD}/build/server/ build/server/configlib.so build/server/connectionQueuelib.so -o bin/server $(LIBS)

build/server/server.o: src/server/server.c
		$(CC) $(CFLAGS) src/server/server.c -c -I $(SERVER_INCLUDE) -o build/server/server.o

#CREAZIONE LIBRERIA DINAMICA CONFIG
build/server/configlib.so: build/server/configlib.o src/server/includes/config.h
		$(CC) -shared build/server/configlib.o -o build/server/configlib.so

#CREAZIONE LIBRERIA DINAMICA connectionQueue
build/server/connectionQueuelib.so: build/server/connectionQueuelib.o src/server/includes/connectionQueue.h
		$(CC) -shared build/server/connectionQueuelib.o -o build/server/connectionQueuelib.so

# COMPILAZIONE LIBRERIA config
build/server/configlib.o: src/server/includes/config.c
		$(CC) $(CFLAGS) $(SERVER_INCLUDE)/config.c -c -fPIC -I $(SERVER_INCLUDE) -o build/server/configlib.o

# COMPILAZIONE LIBRERIA connectionQueue
build/server/connectionQueuelib.o: src/server/includes/connectionQueue.c
		$(CC) $(CFLAGS) $(SERVER_INCLUDE)/connectionQueue.c -c -fPIC -I $(SERVER_INCLUDE) -o build/server/connectionQueuelib.o


#### CLIENT ####

bin/client: build/client/client.o
		$(CC) $(CFLAGS) build/client/client.o -Wl,-rpath=${PWD}/build/client/ build/client/utilslib.so -o bin/client

build/client/client.o: src/client/client.c
		$(CC) $(CFLAGS) src/client/client.c -c -I $(SERVER_INCLUDE) -o build/client/client.o

#CREAZIONE LIBRERIA DINAMICA UTILS
build/client/utilslib.so: build/client/utilslib.o src/client/includes/utils.h
		$(CC) -shared build/client/utilslib.o -o build/client/utilslib.so

# COMPILAZIONE LIBRERIA utils del client
build/client/utilslib.o: src/client/includes/utils.c
		$(CC) $(CFLAGS) $(CLIENT_INCLUDE)/utils.c -c -fPIC -I $(CLIENT_INCLUDE) -o build/client/utilslib.o

all: $(TARGETS)

clean:
	rm -rf bin/*
	rm -rf build/server/*
	rm -rf build/client/*

test_server:
	./bin/server

test_client:
	./bin/client -h