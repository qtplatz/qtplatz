import sqlite3
import matplotlib.pyplot as plt

#conn = sqlite3.connect('/disk2/data/mouse/2018-09-07/Soil_Closed15_0100.adfs')
conn = sqlite3.connect('/home/toshi/data/mouse/iwai/2018-10-26/noAIR_10mV_2650_0011.adfs')
c = conn.cursor()

c.execute( "SELECT ROUND(peak_intensity/5)*5 as threshold, COUNT(*) FROM peak WHERE peak_time > 205.98e-6 and peak_time < 206.01 GROUP BY threshold" )

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
