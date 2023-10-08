#!/bin/bash

files=$(ls /usr/bin/)    #"ls /usr/bin"的执行结果保存到变量files中

for file in $files   #遍历files这个结果集
do
    if [ -L "/usr/bin/$file" ] ; then    #如果某个文件是连接文件，就打印出一条提示信息
        echo "$file is link file"
    fi
done