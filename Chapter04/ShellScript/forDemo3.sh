#!/bin/bash

sum=0
for ((i=1;i<=100;i+=1)) #i从1开始，只要i的值小于等于100就一直执行循环语句，i每次递增1
do
    sum=$(($sum+$i))    #sum用于累计i的和
done
echo "sum=$sum"