#!/bin/bash

if [ $# -ne 1 ] ; then
    echo "please input one interger"
    exit 1
fi

#输入的第一个整数参数是否大于等于10
if [ "$1" -ge 10 ] ; then
    echo "$1 >= 10"
elif [ "$1" -ge 5 ] ; then #输入输入的第一个整数参数是否大于等于5且小于10
    echo "5 <= $1 < 10"
else
    echo "$1 < 5"
fi