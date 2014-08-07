
set multiplot
set grid
set xrange [12:34]
plot "spectrum.txt" using 2:3 with lines title "original" axes x1y2 , \
     "spectrum.txt" using 2:4 with lines title "derivative" axes x1y1, \
     "spectrum.txt" using 2:5 with lines title "smoothed" axes x1y2

set nomultiplot


