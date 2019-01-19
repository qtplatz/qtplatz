
reset

set terminal epslatex size 6in,4in color colortext standalone header \
"\\usepackage{graphicx}\n\\usepackage{amsmath}\n\\usepackage[version=3]{mhchem}\n\\usepackage{siunitx}"
#set terminal x11
#set terminal postscript eps enhanced color size 8in,11.5in font 'Helvetica,24' linewidth 3
#set terminal epslatex size 10in,8in color colortext
#set terminal svg size 1280 720 fname 'Helvetica' fsize 16

set output ARG1
dbfile = ARG2

set datafile separator "|"

set multiplot layout 2,1
#------------------------------ frequency plot ----------------------------------
set title sprintf("Single ion peak height/frequency")
set xlabel "Amplitude(mV)"
set ylabel "Frequency"

set boxwidth 2 absolute
set style fill solid 1.0 noborder

plot '< sqlite3 ' . dbfile . ' \
"SELECT round(peak_intensity/5)*5 as threshold, count(*) FROM peak WHERE ((338.0e-6 < peak_time AND peak_time < 360.0e-6) OR (370.0e-6 < peak_time)) GROUP BY threshold"' \
     using 1:2 smooth frequency with boxes notitle \
     , '< sqlite3 ' . dbfile . ' \
"SELECT round(peak_intensity/5)*5 as threshold, count(*) FROM peak WHERE ((338.0e-6 < peak_time AND peak_time < 360.0e-6) OR (370.0e-6 < peak_time)) GROUP BY threshold"' \
     using 1:2 with linespoints title "Time range: $[338.0, 360\\mu s]$, $370.0, 386.0\\mu s]$"

set xrange[0:100]

plot '< sqlite3 ' . dbfile . ' \
"SELECT round(peak_intensity/5)*5 as threshold, count(*) FROM peak WHERE (347.941e-6 < peak_time AND peak_time < 347.961e-6) GROUP BY threshold"' \
     using 1:2 smooth frequency with boxes notitle \
     , '< sqlite3 ' . dbfile . ' \
"SELECT round(peak_intensity/5)*5 as threshold, count(*) FROM peak WHERE (347.941e-6 <= peak_time AND peak_time <= 347.961e-6) GROUP BY threshold"' \
     using 1:2 with linespoints title "Time range: 347.951 $\\pm 0.010\\mu s$"
