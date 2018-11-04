#!/usr/local/bin/python3
import sqlite3
import matplotlib.pyplot as plt

#conn = sqlite3.connect('/disk2/data/mouse/2018-09-07/Soil_Closed15_0100.adfs')
conn = sqlite3.connect('/Users/toshi/data/mouse/100ppmN2O_CO2_50ul_0001.adfs')
c = conn.cursor()

c.execute( "SELECT ROUND((timeSinceEpoch-(SELECT MIN(timeSinceEpoch) FROM trigger))/1e9) AS time,COUNT(*) \
 FROM trigger,peak WHERE id=idTrigger AND protocol=1 GROUP BY time" )

result_set = c.fetchall()

time = []
count = []

for row in result_set:
    time.append( row[0] )
    count.append( row[1] )

plt.ylabel('Ion counts')
plt.xlabel('Time(seconds)')
plt.grid( True )

plt.scatter( time, count )
plt.plot( time, count )

plt.show()
