#!/bin/bash

i=9    #初始化i的值为9

until [ $i -le 0 ]    #i的值小于等于0就停止循环
do
    j=1    #初始化j的值为1
    until [ $j -gt $i ]    #j的值大于i的值就停止内层循环
    do
        temp=$(($j*$i))
        echo -n "$i * $j = $temp  "
        j=$(($j+1))    #j的值累计加1
    done
    i=$(($i-1))    #i的值累计减1
    echo
done