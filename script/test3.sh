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
echo $start_time

#Loop for 30 sec
while [ $(($(date +%s)-$start_time)) -lt 3 ]; do
  echo "ciao"
done
#lancio server
#${SERVER} &
#PID_SERVER=$!
#
#${CLIENT} -h
#
##Lancio client 1
#${CLIENT} -f ${SOCKET} -p -D test/test1/expelled -w test/test1/folder/
#
##Lancio client 2
#${CLIENT} -f ${SOCKET} -p -D test/test1/expelled -W test/test1/folder/test1.txt,test/test1/folder/test2.txt
#
##Lancio client 3
#${CLIENT} -f ${SOCKET} -p -d test/test1/read/ -r test/test1/folder/test1.txt,test/test1/folder/test2.txt
#
##Chiudo il server
#kill -SIGINT ${PID_SERVER}