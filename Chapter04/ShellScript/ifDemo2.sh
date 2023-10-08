#!/bin/bash

if [ $# -ne 1 ] ; then
    echo "please input one directory"
    exit 1
fi

#判断用户输入的第一次参数是否是一个目录
if [ -d "$1" ] ; then echo "$1 is directory" ; else echo "$1 is not directory" ; fi