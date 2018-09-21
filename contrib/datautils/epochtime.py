#!/usr/bin/python
# See: https://docs.python.org/2/library/datetime.html#strftime-and-strptime-behavior
import sqlite3
import time
from datetime import datetime

conn = sqlite3.connect('timestamp.db')
c = conn.cursor()
c.execute( "select * from results" )

result_set = c.fetchall()

ftr = [3600,60,1]
prev_time_of_inject_since_epoch = 0
prev_posix_time_of_startrun = 0
print "\"file\"\t\"created\"\t\"startrun\"\t\"inject\", \"injection-interval\", \"ctime interval\""

for row in result_set:
    # print "%s, %s, %f" % ( row[0], row[1], row[2]*1e-9 )
    time_of_inject_since_epoch = row[2] / 1.0e9 # epoch_time := value taken from AcquiredData (seconds)
    posix_time_of_midnight = time.mktime( datetime.strptime( row[1].split()[0], "%Y-%b-%d" ).timetuple() )
    day_time_of_startrun = sum( [ a*b for a,b in zip(ftr, map( float, row[1].split()[1].split(':') )) ] )
    posix_time_of_startrun = posix_time_of_midnight + day_time_of_startrun
    tp_startrun = datetime.fromtimestamp( posix_time_of_startrun )
    duration_inject = time_of_inject_since_epoch - prev_time_of_inject_since_epoch
    duration_ctime  = posix_time_of_startrun - prev_posix_time_of_startrun
    print "\"%s\"\t\"%s\"\t%.3f\t%.3f\t%3f\t%3f" %  ( row[0]
                                                      , tp_startrun
                                                      , posix_time_of_startrun
                                                      , time_of_inject_since_epoch
                                                      , duration_inject
                                                      , duration_ctime )
    prev_time_of_inject_since_epoch = time_of_inject_since_epoch
    prev_posix_time_of_startrun = posix_time_of_startrun

