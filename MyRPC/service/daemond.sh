#!/bin/bash

# 加载系统封装好的函数
source /etc/init.d/functions

GREEN='\033[1;32m' # 绿色
RES='\033[0m'

SRV=$(basename ${BASH_SOURCE})
# 程序绝对路径
PROG="/home/backend/service/$SRV/$SRV"
# 服务锁文件
LOCK_FILE="/home/backend/lock/subsys/$SRV"

RET=0

# 服务启动函数
function start() {
    mkdir -p /home/backend/lock/subsys
    # 锁文件不存在说明服务没启动
    if [ ! -f $LOCK_FILE ]; then
        echo -n $"Starting $PROG: "
        # 启动服务，success和failure是系统封装的shell函数，它会在终端打印“[  OK  ]”，failure则会打印“[FAILED]”
        $PROG -d && success || failure
        RET=$?
        echo
    else
        LOCK_FILE_PID=$(cat $LOCK_FILE)
        # 获取服务进程id，如果获取成功表明服务已经在运行，否则启动服务
        PID=$(pidof $PROG | tr -s ' ' '\n' | grep $LOCK_FILE_PID)
        if [ ! -z "$PID" ] ; then
            echo -e "$SRV(${GREEN}$PID${RES}) is already running…"
        else
            # 进程id不存在，启动服务
            echo -n $"Starting $PROG: "
            $PROG -d && success || failure
            RET=$?
            echo
        fi
    fi
    return $RET
}

# 服务停止函数
function stop() {
    # 锁文件不存在说明服务已经停止了
    if [ ! -f $LOCK_FILE ]; then
        echo "$SRV is stopd"
        return $RET
    else
        echo -n $"Stopping $PROG: "
        # killproc是系统封装好的shell函数用于停止服务
        killproc $PROG
        RET=$?
        # 删除锁文件
        rm -f $LOCK_FILE
        echo
        return $RET
    fi
}

# 服务重启函数
function restart() {
    stop    # 先停止服务
    start   # 重启启动服务
}

# 查看服务状态函数
function status() {
    if [ ! -f $LOCK_FILE ] ; then
        echo "$SRV is stoped"
    else
        LOCK_FILE_PID=$(cat $LOCK_FILE)
        PID=$(pidof $PROG | tr -s ' ' '\n' | grep $LOCK_FILE_PID)
        if [ -z "$PID" ] ; then
            echo "$SRV dead but locked"
        else
            echo -e "$SRV(${GREEN}$PID${RES}) is running…"
        fi
    fi
}

case "$1" in
start)
    start
    ;;
stop)
    stop
    ;;
restart)
    restart
    ;;
status)
    status
    ;;
*)
    echo $"Usage: $0 {start|stop|restart|status}"
    exit 1
esac

exit $RET
