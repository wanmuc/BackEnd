#!/bin/bash
PROG=$(pwd | xargs basename)
cp -f ./conf/${PROG}_client.conf /home/backend/route # 发布路由文件