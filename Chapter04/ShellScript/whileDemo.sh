#!/bin/bash

i=1    #初始化i的值为1

while [ $i -le 9 ]    #只要i的值小于等于9循环就一直执行
do
    j=1
    while [ $j -le $i ]    #内层循环，只要j小于等于i的值就一直执行
    do
        temp=$(($i*$j))    #计算i*j的值保存到temp变量中，在shell脚本中$(())可以做算数运算
        echo -n "$i * $j = $temp  "
        j=$(($j+1))  #j的值累计加1
    done
    i=$(($i+1))    #i的值累计加1
    echo
done