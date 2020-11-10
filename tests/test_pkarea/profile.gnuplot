set terminal epslatex size 9.5in,6in color colortext standalone header \
"\\usepackage{graphicx}\n\\usepackage{amsmath}\n\\usepackage[version=3]{mhchem}\n\\usepackage{siunitx}"

set output ARG1

#command line: gnuplot -c profile.gnuplot

set multiplot layout 3,1

cmd = '/Users/toshi/src/build-Darwin-i386/qtplatz.release/bin/test_pkarea'

set xlabel "time-offset(ns)"
set ylabel "\\si{percent} area"

plot '< ' . cmd . ' --fraction --replicates=10' using ($$6+($$7-$$6)/2):($$8*100) with linespoints


set xlabel "time-of-flight"
set ylabel "Intensity"

plot '< ' . cmd . " " using 1:2 with linespoints \
     , '< ' . cmd . ' --fence --replicates=0' using 1:2 with impulse

plot '< ' . cmd . " " using 1:2 with linespoints \
     , '< ' . cmd . ' --fence --replicates=10' using 1:2 with impulse

pause mouse
