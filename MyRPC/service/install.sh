#!/bin/bash
SERVICE=$(ls | grep -v sh | grep -v redis)
for SVR in $SERVICE
do
    cd ./$SVR
    ./install.sh
    cd -
done
