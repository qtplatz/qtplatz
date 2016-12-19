#!/bin/sh
### BEGIN INIT INFO
# Provides:          acqirisd
# Required-Start:    $all
# Required-Stop:     $all
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: acqirisd
# Description:       acqirisd daemon
### END INIT INFO

case "$1" in
  start)
      echo -n "Starting digital oscilosope daemon: acqirisd"
      /opt/qtplatz/bin/acqirisd --server --daemon > /dev/null 2>&1 &
      echo "."
      ;;
  stop)
      echo -n "Stopping digital oscilosope daemon: acqirisd"
      kill -HUP `cat /var/run/acqirisd.pid`
      echo "."
      ;;
  restart)
      echo -n "Restarting digital osciloscope daemon: acqirisd"
      kill -HUP `cat /var/run/acqirisd.pid`
      sleep 3
      /opt/qtplatz/bin/acqirisd --server --daemon > /dev/null 2>&1 &
      echo "."
      ;;

  *)
      echo "Usage: /etc/init.d/acqirisd {start|stop|restart}"
      exit 1
esac

exit 0
