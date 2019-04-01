#set terminal postscript eps enhanced color size 11.5in,8.0in font 'Helvetica,24' linewidth 3
set terminal epslatex size 12in,7in color colortext standalone header \
"\\usepackage{graphicx}\n\\usepackage{amsmath}\n\\usepackage[version=3]{mhchem}"

set output ARG1
input = ARG2
caption = ARG3
set multiplot layout 2,1

print sprintf("********************** Time-of-flight/Peak Heights '%s', ARGC=%d", input, ARGC )
print sprintf("********************** Time-of-flight/Peak Heights '%s'", caption )

set title sprintf("Time-of-flight/Peak Heights '%s'", caption )

tof_lower = 99.90e-6
tof_upper = 99.92e-6

SQL = sprintf( "'\
SELECT *,COUNT(*) AS COUNTS FROM (SELECT MIN(peak_time),ROUND(peak_intensity/10)*10 AS Threshold \
FROM trigger,peak WHERE id=idTrigger AND peak_time > %.14g AND peak_time < %.14g GROUP BY id) \
GROUP BY Threshold'", tof_lower, tof_upper )

SQL1 = sprintf( "'\
SELECT Threshold,COUNT(*) AS COUNTS FROM (SELECT ROUND(peak_intensity/10)*10 AS Threshold \
FROM trigger,peak WHERE id=idTrigger AND peak_time > %.14g AND peak_time < %.14g) \
GROUP BY Threshold'", tof_lower, tof_upper )


set ylabel "Frequence(counts)"
set xlabel "Peak height(-mV)"

set datafile separator "|"

#plot '< sqlite3 '. input . " " . SQL using (-$2):3 with impulses pt 4 ps 2.5 axis x1y1 title ""
plot '< sqlite3 '. input . " " . SQL using (-$2):3 with linespoints pt 4 ps 2.5 axis x1y1 title "exclude following peaks"

set notitle
plot '< sqlite3 '. input . " " . SQL1 using (-$1):2 with linespoints pt 4 ps 2.5 axis x1y1 title "all peaks"
