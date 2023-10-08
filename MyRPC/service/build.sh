#!/bin/bash
SERVICE=$(ls | grep -v sh | grep -v redis)
for SVR in $SERVICE
do
    cd ./$SVR
    ./build.sh
    cd -
done

