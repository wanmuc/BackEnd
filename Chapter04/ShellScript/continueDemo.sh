#!/bin/bash

sum=0
for i in $(seq 1 100)   #seq命令用于生成1-100的序列，for循环就可以遍历这个序列
do
    if [ $i -eq 10 ] ; then  # i等于10则直接跳过
        continue
    fi
    sum=$(($i+$sum))    #sum用于累计i的和
done
echo "sum=$sum"