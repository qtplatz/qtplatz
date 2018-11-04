#!/usr/local/bin/python3
import sqlite3

#conn = sqlite3.connect('/disk2/data/mouse/2018-09-07/Soil_Closed15_0100.adfs')
conn = sqlite3.connect('/Users/toshi/data/mouse/100ppmN2O_CO2_50ul_0001.adfs')
c = conn.cursor()

#c.execute( "SELECT ROUND(peak_intensity/5)*5 as threshold, avg(peak_intensity), COUNT(*) FROM peak GROUP BY threshold" )
c.execute( "SELECT ROUND(peak_intensity/5)*5 as threshold, COUNT(*) FROM peak GROUP BY threshold" )

result_set = c.fetchall()

for row in result_set:
    print ( row )
