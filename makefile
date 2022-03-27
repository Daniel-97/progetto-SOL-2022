
CC = gcc
#D_POSIX_C_SOURCE server per timespec, -lm per linkare math.h
CFLAGS = -std=c99 -Wall -pedantic -g -D_POSIX_C_SOURCE=200112L -D_DEFAULT_SOURCE
LIBS = -lpthread

SERVER_INCLUDE = src/server/includes
CLIENT_INCLUDE = src/client/includes

TARGETS = 	bin/server	\
			bin/client

.PHONY: all clean test1 test2 test3
.SUFFIXES: .c .h #Necessario per notazione %.c


bin/server: src/server/*.c
		$(CC) $(CFLAGS) src/server/*.c -o bin/server $(LIBS)

bin/client: src/client/*.c
		$(CC) $(CFLAGS) src/client/*.c -o bin/client -lm

.DEFAULT_GOAL := all

all: $(TARGETS)

clean:
	rm -rf bin/*

test1:
	./script/test1.sh
test2:
	./script/test2.sh
test3:
	./script/test3.sh
