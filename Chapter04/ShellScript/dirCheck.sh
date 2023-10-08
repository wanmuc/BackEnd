#!/bin/bash

#read命令从终端获取输入保存到dir变量中，p选项表示先在终端显示提示信息"please input the directory name : "
read -p "please input the directory name : " dir
#判断dir是否存在，如果不存在则提示错误信息并退出
test ! -e $dir && echo "$dir is not exist" && exit 1
#判断dir是否真的为目录，如果不是目录则提示错误信息并退出
test ! -d $dir && echo "$dir is not a directory" && exit 1
#判断当前用户是否有读目录的权限
test -r $dir && echo "$USER can read the $dir"
#判断当前用户是否有写目录的权限
test -w $dir && echo "$USER can write the $dir"
#判断当前用户是否有执行目录的权限
test -x $dir && echo "$USER can execution the $dir"