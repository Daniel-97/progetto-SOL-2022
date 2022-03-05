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
#client_args=("-W test/test2/imgs/img2.jpg")
client_args=("-r test/test2/imgs/img2.jpg" "-W test/test2/test1.txt" "-W test/test2/imgs/img2.jpg")

#START THE SERVER
#${SERVER} &
#PID_SERVER=$!

#LOOP FOR 30 SEC
while [ $(($(date +%s)-$start_time)) -lt 10 ]; do

  #Loop through the array of client pid.
  for (( i=0; i<10; i++))
  do
    # If the pid is -1 i need to start a new client process
    # else i need to check if the old client is still running
    if [ "${clients_pid[$i]}" -eq -1 ];
    then
        #Get a random arguments for the client
        rand=$(($RANDOM % ${#client_args[@]}))
#        ${CLIENT} -p -f ${SOCKET} -t 0 -D test/test3/expelled -d test/test3/read -W test/test2/imgs/img2.jpg &
        ${CLIENT} -p -f ${SOCKET} -t 0 -D test/test3/expelled -d test/test3/read ${client_args[$rand]} &
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

##Chiudo il server
#kill -SIGINT ${PID_SERVER}

#Force close all clients
#pkill -f ./bin/client