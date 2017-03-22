reset
#set terminal x11
set terminal postscript eps enhanced color size 10in,8in font 'Helvetica,24' linewidth 3
#set terminal epslatex size 10in,8in color colortext
#set terminal svg size 1280 720 fname 'Helvetica' fsize 16
set output 'count_frequency.eps'
set title sprintf("Count/Frequency")
set xlabel "Intensity(-mV)"
set ylabel "Relative Frequency (%)" # (^4He^{2+})"
set y2label "Average peak width/height(ns/mV)"
set y2tics
set xrange [0:200]
set y2range [0:0.2]

input = 'frequency.db'
#set yrange [500:2000]
#set key outside

set datafile separator "|"

# space before \ backslash in first line is significant
stats '< sqlite3 ' . input . ' \
"SELECT threshold,sum(counts) FROM frequency WHERE threshold < -5.0 AND protocol=0 GROUP BY threshold"' \
      using (-$1):($2) name "p0"

stats '< sqlite3 ' . input . ' \
"SELECT threshold,sum(counts) FROM frequency WHERE threshold < -5.0 AND protocol=1 GROUP BY threshold"' \
      using (-$1):($2) name "p1"

plot '< sqlite3 ' . input . ' \
"SELECT threshold,sum(counts) FROM frequency WHERE threshold < -5.0 AND protocol=0 GROUP BY threshold"' \
     using (-$1):($2*100/p0_max_y) with linespoints pt 4 ps 2.5 axis x1y1 title "p0 frequency" \
     , '< sqlite3 ' . input . ' \
"SELECT threshold,sum(counts) FROM frequency WHERE threshold < -5.0 AND protocol=1 GROUP BY threshold"' \
     using (-$1):($2*100/p1_max_y) with linespoints pt 5 ps 2.5 axis x1y1 title "p1 frequency" \
     , '< sqlite3 ' . input . ' \
"SELECT threshold,avg(average_peak_width),avg(average_peak_intensity) \
FROM frequency WHERE threshold < -5.0 AND protocol=0 GROUP BY threshold"' \
       using (-$1):(-$2/$3) with linespoints pt 6 ps 2.5 axis x1y2 title "p0 width/height" \
     , '< sqlite3 ' . input . ' \
"SELECT threshold,avg(average_peak_width),avg(average_peak_intensity) \
FROM frequency WHERE threshold < -5.0 AND protocol=1 GROUP BY threshold"' \
       using (-$1):(-$2/$3) with linespoints pt 7 ps 2.5 axis x1y2 title "p1 width/height"
