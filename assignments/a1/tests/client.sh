#!/bin/bash

# run this script something like ./client.sh 1 to start a client with username user1 and password password1
# multiple : seq 1 50 | xargs -n 1 -P 50 1 ./client.sh

SERVER_IP="127.0.0.1"
SERVER_PORT=12345

CLIENT_NUMBER=$1

PREV_CLIENT=$((CLIENT_NUMBER - 1))
NEXT_CLIENT=$((CLIENT_NUMBER + 1))

USERNAME="user$CLIENT_NUMBER"
PASSWORD="password$CLIENT_NUMBER"

exec 3<>/dev/tcp/$SERVER_IP/$SERVER_PORT

echo "$USERNAME" >&3
sleep 0.1  

echo "$PASSWORD" >&3
sleep 0.1  

read_responses() {
  while read -r response <&3; do
    echo "Response: $response"
  done
  echo "Called:::"
}

read_responses &

if [ $PREV_CLIENT -gt 0 ]; then
    while read -r response <&3; do
        if ! [ "$response" ]; then
            echo "/msg user$PREV_CLIENT Hello user$PREV_CLIENT from user$CLIENT_NUMBER!\n" >&3
        else 
            break
        fi
        sleep 0.1 
    done
fi

read_responses &

# how to not keep this blocking?

if [ $PREV_CLIENT -gt 0 ]; then
    echo "/msg user$PREV_CLIENT Nice meeting you!$PREV_CLIENT from user$CLIENT_NUMBER!\n" >&3
    sleep 0.1
fi

read_responses &

sleep 10

echo "/exit" >&3

exec 3<&-
exec 3>&-
