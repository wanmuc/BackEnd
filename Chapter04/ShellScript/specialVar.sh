#!/bin/bash

echo "parameters count=$#"
echo "shell script name=$0"

for param in "$*"
do
    echo $param
done

for param in "$@"
do
    echo $param
done

if [ $? == 0 ] ; then
    echo "last shell command execution successful"
else
    echo "last shell command execution failure"
fi