#!/bin/bash
mkdir -p /home/backend/bin #要把这个路径设置到PATH变量中
mkdir -p /home/backend/log/myrpcb
cp -f ./myrpcb /home/backend/bin
chmod +x /home/backend/bin/myrpcb