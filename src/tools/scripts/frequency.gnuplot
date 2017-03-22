reset
#set terminal x11
set terminal postscript eps enhanced color size 10in,8in font 'Helvetica,24' linewidth 3
#set terminal epslatex size 10in,8in color colortext
#set terminal svg size 1280 720 fname 'Helvetica' fsize 16
set output 'count_frequency.eps'
set title sprintf("Count/Frequency")
set xlabel "Intensity(-mV)"
set ylabel "Relative Frequency (%)" # (^4He^{2+})"
set y2label "Average peak width(ns)"
set y2tics
set xrange [0:200]
set y2range [0:6]

input = 'frequency.db'
#set yrange [500:2000]
#set key outside

set datafile separator "|"

#stats '< sqlite3 '. input .'\
#"SELECT threshold,sum(counts) FROM frequency WHERE threshold < -5.0 AND protocol=0 GROUP BY threshold"' \
#      using (-$1):($2) name "p0"

plot '< sqlite3 ' . input . ' \
"SELECT threshold \
,sum(counts) \
,avg(average_peak_time) \
,avg(average_peak_intensity) \
,avg(average_peak_width) FROM frequency WHERE threshold < -5.0 AND protocol=0 GROUP BY threshold"' \
     using (-$1):($2) with linespoints pt 4 ps 2.5 axis x1y1 title "p1 frequency"
