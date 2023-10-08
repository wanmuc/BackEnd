#!/bin/bash

#函数getsum定义
function getsum()
{
    echo "function name=$0"
    sum=0
    for i in $(seq 1 $1)
    do
        sum=$(($sum+$i))
    done
    echo "input=$1,sum=$sum"
}

#调用getsum函数
getsum 10
getsum 100