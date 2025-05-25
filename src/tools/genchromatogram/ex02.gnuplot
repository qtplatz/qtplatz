set terminal epslatex size 11.7*1.2,8.3*0.5 in color colortext standalone header \
"\\usepackage{graphicx}\n\\usepackage{amsmath}\n\\usepackage[version=3]{mhchem}\n\\usepackage{siunitx}\n\\DeclareSIUnit\\count{c}\n\
\\sisetup{inter-unit-product=\\ensuremath{{\\cdot}}}"

set output ARG1
EXEC=ARG2

set datafile separator ",|\t"
set border 3 front ls 101 lw 4 lc "black"
set tics nomirror out scale 0.75

#set multiplot layout 1,3 title ""

set xzeroaxis lw 2
set yzeroaxis lw 2

set xlabel "Retention time (s)"
set ylabel "Intensity (counts)"

set label 1 center at first 10.0, 200 '$t_0$' offset 0,1
set arrow 1 from 10,200 to 10,100 lc "red" lw 2

set label 2 center at first 15.33,1050 "Peak 1" offset 0,1
set label 3 center at first 16.765,850 "Peak 2" offset 0,1

plot '< ' . EXEC . " --resolution=2" using 1:2 with lines lw 5 lc "black" notitle
