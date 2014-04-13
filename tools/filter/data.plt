
set multiplot

plot "data.txt" using 1:2 title "original", \
     "data.txt" using 1:3 title "smoothed", \
     "data.txt" using 1:4 title "1st derivertive" axes x1y2

set nomultiplot