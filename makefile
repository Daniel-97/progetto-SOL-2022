
CC = gcc
CFLAGS = -std=c99 -Wall -pedantic -g -D_POSIX_C_SOURCE=200112L -D_DEFAULT_SOURCE
#CFLAGS = -Wall -pedantic -g
LIBS = -lpthread

SERVER_INCLUDE = src/server/includes
CLIENT_INCLUDE = src/client/includes

TARGETS = 	bin/server	\
			bin/client

.PHONY: all clean test1 test2 test3
.SUFFIXES: .c .h #Necessario per notazione %.c

#### SERVER ####

bin/server: src/server/*.c
		$(CC) $(CFLAGS) src/server/*.c -o bin/server $(LIBS)

#### CLIENT ####

#D_POSIX_C_SOURCE server per timespec, -lm per linkare math.h
bin/client: src/client/*.c
		$(CC) $(CFLAGS) src/client/*.c -o bin/client -lm

all: $(TARGETS)

clean:
	rm -rf bin/*
	rm -rf build/server/*
	rm -rf build/client/*

test_server:
	./bin/server

test_client:
	./bin/client

test1:
	./script/test1.sh
test2:
	./script/test2.sh
test3:
	./script/test3.sh
