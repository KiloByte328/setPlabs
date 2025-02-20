#!/bin/bash
if [ $# -eq 0 ]
then
    NUMBER=20
else
    NUMBER=$1
fi
for (( i=1; i <= NUMBER; i++ ))
do
    PORT=43507
    echo $i $PORT 
    nohup ./testcli $PORT $i >> log.txt &
done