#!/bin/bash

CONFIG=./config/config.txt
SERVER=./bin/server
CLIENT=./bin/client
SOCKET=./connection.sk

N_WORKER=8
N_FILE=100
STORAGE_SIZE=32000000

#Scrivo le configurazioni nel file
echo -e "socket-path:${SOCKET}\nthread-workers:${N_WORKER}\nmax-mem-size:${STORAGE_SIZE}\nmax-file:${N_FILE}\nlog-file:./log.txt" > ${CONFIG}

start_time=$(date +%s)
clients_pid=(-1 -1 -1 -1 -1 -1 -1 -1 -1 -1)
client_args=("-W test/test2/imgs/img2.jpg -r test/test2/imgs/img2.jpg -W test/test2/imgs/img3.jpg -r test/test2/imgs/img4.jpg -r test/test2/imgs/img5.jpg" )

#START THE SERVER
${SERVER} &
PID_SERVER=$!

sleep 1

#LOOP FOR 30 SEC
while [ $(($(date +%s)-$start_time)) -lt 30 ]; do

  #Loop through the array of client pid.
  for (( i=0; i<10; i++))
  do
    # If the pid is -1 i need to start a new client process
    # else i need to check if the old client is still running
    if [ "${clients_pid[$i]}" -eq -1 ];
    then
        ${CLIENT} -f ${SOCKET} -t 0 -D test/test3/expelled -d test/test3/read ${client_args} &
        echo "Starting new client with pid $!"
        clients_pid[$i]=$(($!))
    else

      #If the old process is not running set the pid to -1
      ps -p ${clients_pid[$i]} > /dev/null
      if [ $? -eq 1 ];
      then
        echo "Client $((clients_pid[$i])) terminato"
        clients_pid[$i]=-1
      fi

    fi

  done

done

#Force close all clients
#pkill -f ./bin/client

# -2 = SIGINT
##Chiudo il server
kill -2 ${PID_SERVER}