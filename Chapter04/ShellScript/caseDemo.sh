#!/bin/bash

#从终端的标准输入读取用户的输入保存到gender变量中
read -p "please input you gender : " gender

case $gender in
    "female") #匹配"female"
        echo "your select gender is female"
        ;;
    "male") #匹配"male"
        echo "your select gender is male"
        ;;
    *) #输入有误，输出脚本使用提示
        echo "Usage $0 {female|male}"
        ;;
esac