#!/bin/sh
### BEGIN INIT INFO
# Provides:          httpd
# Required-Start:    $all
# Required-Stop:     $all
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: httpd
# Description:       http daemon
### END INIT INFO

case "$1" in
  start)
        echo -n "Starting delay-generator daemon: httpd"
	    /opt/bin/httpd --port 80 --doc_root /opt/html > /dev/null 2>&1 &
        echo "."
	;;
  stop)
        echo -n "Stopping delay-generator daemon: httpd"
	    kill -HUP `cat /var/run/httpd.pid`
        echo "."
	;;
  restart)
        echo -n "Restarting delay-generator daemon: httpd"
	    kill -HUP `cat /var/run/httpd.pid`
	    sleep 3
	    /opt/bin/httpd --port 80 --doc_root /opt/html > /dev/null 2>&1 &
        echo "."
        ;;

  *)
	echo "Usage: /etc/init.d/httpd {start|stop|restart}"
	exit 1
esac

exit 0
