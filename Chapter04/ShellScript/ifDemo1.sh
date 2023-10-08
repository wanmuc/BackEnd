#!/bin/bash

if [ $# -ne 1 ] ; then
    echo "please input one file"
    exit 1
fi

if ls -lrt $1 ; then
    echo "$1 is exist"
fi