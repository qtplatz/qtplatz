#!/bin/sh
### BEGIN INIT INFO
# Provides:          acqiris
# Required-Start:    $all
# Required-Stop:     $all
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: httpd
# Description:       http daemon
### END INIT INFO

case "$1" in
  start)
      echo -n "Starting digital oscilosope daemon: acqiris"
      /opt/qtplatz/bin/acqiris --server --daemon > /dev/null 2>&1 &
      echo "."
      ;;
  stop)
      echo -n "Stopping digital oscilosope daemon: acqiris"
      kill -HUP `cat /var/run/acqiris.pid`
      echo "."
      ;;
  restart)
      echo -n "Restarting digital osciloscope daemon: httpd"
      kill -HUP `cat /var/run/acqiris.pid`
      sleep 3
      /opt/qtplatz/bin/acqiris --server --daemon > /dev/null 2>&1 &
      echo "."
      ;;

  *)
      echo "Usage: /etc/init.d/acqiris {start|stop|restart}"
      exit 1
esac

exit 0
