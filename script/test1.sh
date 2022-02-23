CONFIG=./config/config1.txt
SERVER=./bin/server
CLIENT=./bin/client
SOCKET=./connection.sk

N_WORKER=1
N_FILE=10000
STORAGE_SIZE=128000000

#Scrivo le configurazioni nel file
echo -e "socket-path:${SOCKET}\nthread-workers:${N_WORKER}\nmax-mem-size:${STORAGE_SIZE}\nmax-file:${N_FILE}\nlog-file:./log.txt" > ${CONFIG}

#lancio server
valgrind --leak-check=full ${SERVER} &
PID_SERVER=$!

#Lancio client 1
${CLIENT} -f ${SOCKET} -p -t 200