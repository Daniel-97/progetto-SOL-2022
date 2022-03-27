#!/bin/bash

CONFIG=./config/config.txt
SERVER=./bin/server
CLIENT=./bin/client
SOCKET=./connection.sk

N_WORKER=4
N_FILE=10000
STORAGE_SIZE=128000000

#Scrivo le configurazioni nel file
echo -e "socket-path:${SOCKET}\nthread-workers:${N_WORKER}\nmax-mem-size:${STORAGE_SIZE}\nmax-file:${N_FILE}\nlog-file:./log.txt" > ${CONFIG}

#lancio server
valgrind --leak-check=full ${SERVER} &
PID_SERVER=$!
sleep 1
${CLIENT} -h

#Lancio client 1
${CLIENT} -f ${SOCKET} -p -t 200 -D test/test1/expelled -w test/test1/folder/

#Lancio client 2
${CLIENT} -f ${SOCKET} -p -t 200 -D test/test1/expelled -W test/test1/folder/test1.txt,test/test1/folder/test2.txt

#Lancio client 3
${CLIENT} -f ${SOCKET} -p -t 200 -d test/test1/read/ -r test/test1/folder/test1.txt,test/test1/folder/test2.txt

# -1 = SIGHUP
#Chiudo il server
kill -1 ${PID_SERVER}