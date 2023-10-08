#!/bin/bash

# getopts使用的方法：字母后面带:的表示该选项有对应的参数，如：-d xxx 使用$OPTARG变量可以读取这个xxx参数的值。
# getopts会将输入的-b -d选项分别赋值给arg变量，以便后续处理。
while getopts "bd:" arg
do
    case $arg in
	b)
	echo "set the b options"
	;;
	d)
	echo "d option's parameter is $OPTARG"
	;;
	?)
	echo "$arg :no support this arguments!"
    esac
done