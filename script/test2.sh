#!/bin/bash

CONFIG=./config/config.txt
SERVER=./bin/server
CLIENT=./bin/client
SOCKET=./connection.sk

N_WORKER=1
N_FILE=10
STORAGE_SIZE=1000000

#Scrivo le configurazioni nel file
echo -e "socket-path:${SOCKET}\nthread-workers:${N_WORKER}\nmax-mem-size:${STORAGE_SIZE}\nmax-file:${N_FILE}\nlog-file:./log.txt" > ${CONFIG}

#lancio server
${SERVER} &
PID_SERVER=$!

${CLIENT} -h

#Lancio client 2
${CLIENT} -f ${SOCKET} -p -t 200 -D test/test2/expelled -w test/test2/imgs/

#Lancio client 3
${CLIENT} -f ${SOCKET} -p -t 200 -d test/test2/read/ -r test/test2/imgs/img3.jpg,test/test2/imgs/img4.jpg

# -1 = SIGHUP
#Chiudo il server
kill -1 ${PID_SERVER}