#!/usr/bin/python
import sqlite3
import matplotlib.pyplot as plt

conn = sqlite3.connect('/disk2/data/mouse/2018-09-07/Soil_Closed15_0100.adfs')
c = conn.cursor()
#c.execute( "SELECT ROUND(peak_intensity/5)*5 as threshold, avg(peak_intensity), COUNT(*) FROM peak GROUP BY threshold" )
c.execute( "SELECT ROUND(peak_intensity/5)*5 as threshold, COUNT(*) FROM peak GROUP BY threshold" )

result_set = c.fetchall()

plt.plot( result_set )
plt.show()
