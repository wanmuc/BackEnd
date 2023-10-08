#!/bin/bash
mkdir -p /home/backend/bin #要把这个路径设置到PATH变量中
cp -f ./myrpcc /home/backend/bin
chmod +x /home/backend/bin/myrpcc