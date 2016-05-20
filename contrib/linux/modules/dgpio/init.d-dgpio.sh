#!/bin/sh

### BEGIN INIT INFO
# Provides:          dgpio
# Required-Start:    $local_fs $syslog
# Required-Stop:     $local_fs $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Load kernel module and create device nodes at boot time.
# Description:       Load kernel module and create device nodes at boot time for dgpio instruments.
### END INIT INFO

case "$1" in
    start)
        /sbin/modprobe dgpio
        exit 0
    ;;

    stop)
	/sbin/rmmod dgpio
        exit 0
    ;;

    restart)
        /sbin/rmmod dgpio
        /sbin/modprobe dgpio.ko
        exit 0
    ;;

    *)
        echo "Error: argument '$1' not supported." >&2
        exit 0
    ;;
esac


