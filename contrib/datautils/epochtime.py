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

for row in result_set:
    print "%s, %s, %f" % ( row[0], row[1], row[2]*1e-9 )
    timeOfInj = row[2] / 1.0e9 # epoch_time := value taken from AcquiredData (seconds)
    
    midNight = time.mktime( datetime.strptime( row[1].split()[0], "%Y-%b-%d" ).timetuple() )
    timeStr = row[1].split()[1]
    ctime = sum( [ a*b for a,b in zip(ftr, map( float, timeStr.split(':') )) ] )
    ftime = ctime #+ midNight

    print "%.6f %.6f diff= %.2fh" % ( ftime, timeOfInj, ( ftime - timeOfInj ) / 3600 )
