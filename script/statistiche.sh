#!/bin/bash

#SCRIPT PER LE STATISTICHE DEL SERVER A PARTIRE DAL FILE DI LOG
echo "Reading log file from: $1"

cont_read=$(grep "READ_FILE" -c $1)
cont_n_read=$(grep "READ_N_FILE" -c $1)
cont_write=$(grep "WRITE_FILE" -c $1)
cont_lock=$(grep "LOCK_FILE" -c $1)
cont_unlock=$(grep "UNLOCK_FILE" -c $1)
cont_close=$(grep "CLOSE_FILE" -c $1)
cont_open=$(grep "OPEN_FILE" -c $1)
cont_open_lock=$(grep "OPEN_FILE_LOCK" -c $1)

cont_expelled=0

total_byte_read=0
total_byte_write=0

i=0

while read line; do
  i=$((i+1))
  if [ $i == 1 ]; then
    continue
  fi

  IFS=','
  read -a array <<< $line

  b_read="${array[5]}"
  b_write="${array[6]}"

  if grep -q "WRITE" <<< "$line"; then
    total_byte_write=$((total_byte_write+b_write))
  fi

  if grep -q "READ" <<< "$line" || grep -q "READ_N" <<< "$line"; then
    total_byte_read=$((total_byte_read+b_read))
  fi

  #Conta il numero di file espulsi
  replaced="${array[7]}"
  echo $replaced
  if [ "$replaced" != "null" ]; then
    cont_expelled=$((cont_expelled+1))
  fi

done < $1

echo "****** STATISTICHE SERVER *****"
echo "- cont read: $cont_read"
echo "- cont n_read: $cont_n_read"
echo "- cont write: $cont_write"
echo "- cont lock: $cont_lock"
echo "- cont unlock: $cont_unlock"
echo "- cont close: $cont_close"
echo "- cont open-lock: $((cont_open_lock))"
echo "- cont open: $cont_open"
echo "- total read byte: $total_byte_read"
echo "- total write byte: $total_byte_write"
echo "- total replaced file: $cont_expelled"
