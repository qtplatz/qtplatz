#!gnuplot
#       gnuplot -c timestamp.gnuplot
#       pdflatex timestamp.tex
reset
set terminal epslatex size 6in,6in color colortext standalone header \
"\\usepackage{graphicx}\n\\usepackage{amsmath}\n\\usepackage[version=3]{mhchem}"

infile='timestamp.txt'
set output 'timestamp.tex'
set multiplot layout 2,1

set xlabel "Data number"
set ylabel "Duration(seconds)"

plot infile using 0:5 every ::2 with lines title "file creation" \
     , '' using 0:6 every ::2 with lines title "sample injection"

set yrange[-500:500]
plot infile using 0:5 every ::2 with lines title "file creation" \
     , '' using 0:6 every ::2 with lines title "sample injection"
