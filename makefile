
CC = gcc
CFLAGS = -std=c99 -Wall
LIBS = -pthread

TARGETS = 	bin/server	\
			bin/client

.PHONY: all clean test1 test2 test3

bin/server: build/server/server.o build/server/configlib.so
		$(CC) build/server/server.o -Wl,-rpath=${PWD}/build/server/ build/server/configlib.so -o bin/server

build/server/server.o: src/server/server.c
		$(CC) $(CFLAGS) src/server/server.c -c -I src/server/includes -o build/server/server.o

#CREAZIONE LIBRERIA DINAMICA
build/server/configlib.so: build/server/configlib.o src/server/includes/config.h
		$(CC) -shared build/server/configlib.o -o build/server/configlib.so

# COMPILAZIONE LIBRERIA config. Il parametro -I indica al compilatore dove cercare gli headers
build/server/configlib.o: src/server/includes/config.c src/server/includes/config.h
		$(CC) $(CFLAGS) src/server/includes/config.c -c -fPIC -I src/server/includes -o build/server/configlib.o

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