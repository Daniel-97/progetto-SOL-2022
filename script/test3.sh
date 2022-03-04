CONFIG=./config/config.txt
SERVER=./bin/server
CLIENT=./bin/client
SOCKET=./connection.sk

N_WORKER=8
N_FILE=100
STORAGE_SIZE=32000000

#Scrivo le configurazioni nel file
echo -e "socket-path:${SOCKET}\nthread-workers:${N_WORKER}\nmax-mem-size:${STORAGE_SIZE}\nmax-file:${N_FILE}\nlog-file:./log.txt" > ${CONFIG}

max_client=10
num_client=0
start_time=$(date +%s)

#START THE SERVER
#lancio server
${SERVER} &
PID_SERVER=$!

#LOOP FOR 30 SEC
while [ $(($(date +%s)-$start_time)) -lt 3 ]; do

  if [ $num_client -lt $max_client ]; then

    ${CLIENT} -f ${SOCKET} -t 500 -W test/test2/imgs/img0.jpg
    echo "Starting new client with pid $!"
    num_client=$((num_client+1))

  fi

done

##Chiudo il server
kill -SIGINT ${PID_SERVER}