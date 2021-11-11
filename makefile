
CC = gcc
CFLAGS = -std=c99 -Wall
LIBS = -lpthread

SERVER_INCLUDE = src/server/includes

TARGETS = 	bin/server	\
			bin/client

.PHONY: all clean test1 test2 test3
.SUFFIXES: .c .h #Necessario per notazione %.c

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

# per adesso non usato
#server.o: src/server/server.c
#		$(CC) $(CFLAGS) -c src/server/server.c -o build/object/server/server.o

bin/client:
		$(CC) $(CFLAGS) src/client/client.c -o bin/client $(LIBS)
#per adesso non usato
#client.o: src/client/client.c
#		$(CC) $(CFLAGS) -c src/client/client.c -o build/object/client/client.o

all: $(TARGETS)

clean:
	rm -rf bin/*
	rm -rf build/server/*
	rm -rf build/client/*

test1:
	./bin/server