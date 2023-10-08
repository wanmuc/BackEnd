#!/bin/bash
source /etc/init.d/functions
SERVICE=$(ls /home/backend/script | grep -v service.sh)

function start() {
    for SVR in $SERVICE
    do
        /home/backend/script/$SVR start
    done
}
function stop() {
    for SVR in $SERVICE
    do
        /home/backend/script/$SVR stop
    done
}
function restart() {
    for SVR in $SERVICE
    do
        /home/backend/script/$SVR stop
        /home/backend/script/$SVR start
    done
}
function status() {
    for SVR in $SERVICE
    do
        /home/backend/script/$SVR status
    done
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