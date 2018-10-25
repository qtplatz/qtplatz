#!/usr/local/bin/python3
import sqlite3
import matplotlib.pyplot as plt

#conn = sqlite3.connect('/disk2/data/mouse/2018-09-07/Soil_Closed15_0100.adfs')
conn = sqlite3.connect('/Users/toshi/data/mouse/100ppmN2O_CO2_50ul_0001.adfs')
c = conn.cursor()

c.execute( "SELECT ROUND(peak_intensity/5)*5 as threshold, COUNT(*) FROM peak GROUP BY threshold" )

result_set = c.fetchall()

V = []
counts = []

for row in result_set:
    V.append( -row[ 0 ] )
    counts.append( row[ 1 ] )

plt.ylabel('Frequency(counts)')
plt.xlabel('Peak Height(mV)')

plt.grid( True )

plt.scatter( V, counts )
plt.plot( V, counts )
plt.show()
