#!/bin/bash

# Create 50 dummy users

# read from users.txt.bak line by line
# restore original and then add 50

# empty the file
> ../users.txt

while IFS= read -r line
do
    echo "$line" >> ../users.txt
done < users.txt.bak

for i in {1..50}
do
    echo "user$i:password$i" >> ../users.txt 
done

echo "Added 50 Dummy Users!"
