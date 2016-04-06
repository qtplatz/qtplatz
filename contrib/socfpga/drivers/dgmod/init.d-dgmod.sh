#!/bin/bash

### BEGIN INIT INFO
# Provides:          dgmod
# Required-Start:    $local_fs $syslog
# Required-Stop:     $local_fs $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Load kernel module and create device nodes at boot time.
# Description:       Load kernel module and create device nodes at boot time for dgmod instruments.
### END INIT INFO

case "$1" in
    start)
        /sbin/insmod dgmod.ko
        exit 0
    ;;

    stop)
	/sbin/rmmod dgmod
        exit 0
    ;;

    restart)
        /sbin/rmmod dgmod
        /sbin/insmod dgmod.ko
        exit 0
    ;;

    *)
        echo "Error: argument '$1' not supported." >&2
        exit 0
    ;;
esac


