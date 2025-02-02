#!/bin/bash

SERVER_IP="127.0.0.1"
SERVER_PORT=12345

CLIENT_NUMBER=$1

PREV_CLIENT=$((CLIENT_NUMBER - 2))
NEXT_CLIENT=$((CLIENT_NUMBER + 2))

USERNAME="user$CLIENT_NUMBER"
PASSWORD="password$CLIENT_NUMBER"

exec 3<>/dev/tcp/$SERVER_IP/$SERVER_PORT

timeout 0.1s bash -c '
while read -r response <&3; do
  echo -n ""
done
'
\echo "$USERNAME" >&3
sleep 0.1  

timeout 0.1s bash -c '
while read -r response <&3; do
  echo -n ""
done
'

echo "$PASSWORD" >&3
sleep 0.1  

# timeout 0.2s bash -c '
# while read -r response <&3; do
#   echo -n ""
# done
# '

read_responses() {
  while read -r response <&3; do
    echo "Response: $response"
  done
}

read_responses &

if [ $PREV_CLIENT -gt 0 ]; then
  printf "/msg user$PREV_CLIENT Hello user$PREV_CLIENT from user$CLIENT_NUMBER!\n" >&3
  sleep 0.1  
fi

# how to not keep this blocking?

if [ $PREV_CLIENT -gt 0 ]; then
    for i in {1..5} 
        do
        printf "/msg user$PREV_CLIENT Nice meeting you!$PREV_CLIENT from user$CLIENT_NUMBER!\n" >&3
        sleep 1 
    done
fi

exec 3<&-
exec 3>&-