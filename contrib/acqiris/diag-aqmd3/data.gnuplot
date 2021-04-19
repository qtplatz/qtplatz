# run: gnuplot -c data.gnuplot

input = 'data.txt'

set multiplot layout 2,1

plot input using ($1*1e6):3 with linespoints

plot input using ($1*1e6):4 with linespoints

pause -1