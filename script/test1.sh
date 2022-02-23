CONFIG=./config/config1.txt
SERVERG=./bin/server
CLIENT=./bin/client

N_WORKER=1
N_FILE=10000
STORAGE_SIZE=128000000

#Scrivo le configurazioni nel file
echo -e "socket-path:./connection.sk\nthread-workers:${N_WORKER}\nmax-mem-size:${STORAGE_SIZE}\nmax-file:${N_FILE}\nlog-file:./log.txt" > ${CONFIG}

