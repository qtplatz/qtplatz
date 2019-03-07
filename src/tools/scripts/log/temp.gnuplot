set terminal epslatex size 24in,12in color colortext standalone header \
"\\usepackage{graphicx}\n\\usepackage{amsmath}\n\\usepackage[version=3]{mhchem}\n\\usepackage{siunitx}"

set output ARG1
logfile = ARG2
#if ( ARGC >= 4 ) {
#  set xrange[time(0)+3600*9-ARG4*3600:time(0)+3600*9]
#}

set multiplot layout 2,1

set xtics nomirror rotate
set timefmt "%b %d %H:%M:%S"
#set y2tics
#stats logfile using 1 nooutput

set xdata time
set format x "%b %d %H:%M:%S"
#set xlabel sprintf( "Hours (%s)", strftime("%T",time(0)) )
#set lmargin 12

set ylabel  "Temp.(\\si{\\celsius})"
set format y "%.2f"
set grid
#plot logfile using (($1-STATS_max)+time(0)):4 with linespoints pointinterval 600 pt 5 ps 0.5 axes x1y1 title "T"

set yrange [50:75]

plot 'temp4.log' using 1:9 with linespoints pointinterval 15 pt 5 ps 0.5 title "T" \
     , 'temp3.log' using 1:9 with linespoints pointinterval 15 pt 5 ps 0.5 title "T" \
     , 'temp2.log' using 1:9 with linespoints pointinterval 15 pt 5 ps 0.5 title "T" \
     , 'temp1.log' using 1:9 with linespoints pointinterval 15 pt 5 ps 0.5 title "T" \
     , 'temp0.log' using 1:9 with linespoints pointinterval 15 pt 5 ps 0.5 title "T"

set xrange ['Feb 5 00:00:00':'Feb 6 00:00:00']

plot 'temp4.log' using 1:9 with linespoints pointinterval 15 pt 5 ps 0.5 title "T" \