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

byte_read=0
byte_write=0

while read line; do

  IFS=','
  read -a array <<< $line
  byte="${array[2]}"

  if grep -q "WRITE" <<< "$line"; then
    byte_write=$((byte_write+byte))
  fi

  if grep -q "READ" <<< "$line" || grep -q "READ_N" <<< "$line"; then
    byte_read=$((byte_read+byte))
  fi

done < $1

echo "****** STATISTICHE SERVER *****"
echo "- cont read: $cont_read"
echo "- cont read: $cont_n_read"
echo "- cont write: $cont_write"
echo "- cont lock: $cont_lock"
echo "- cont unlock: $cont_unlock"
echo "- cont close: $cont_close"
echo "- cont open-lock: $((cont_open_lock))"
echo "- cont open: $cont_open"
echo "- total read byte: $byte_read"
echo "- total write byte: $byte_write"
