
reset

set terminal epslatex size 8in,3in color colortext standalone header \
"\\usepackage{graphicx}\n\\usepackage{amsmath}\n\\usepackage[version=3]{mhchem}\n\\usepackage{siunitx}"
#set terminal x11
#set terminal postscript eps enhanced color size 8in,11.5in font 'Helvetica,24' linewidth 3
#set terminal epslatex size 10in,8in color colortext
#set terminal svg size 1280 720 fname 'Helvetica' fsize 16

set output ARG1
dbfile = ARG2

set datafile separator "|"

#set multiplot layout 2,1
#------------------------------ frequency plot ----------------------------------
set title sprintf("Single ion peak height/frequency")
set xlabel "Amplitude(mV)"
set ylabel "Frequency" # (^4He^{2+})"

plot '< sqlite3 ' . dbfile . ' \
"SELECT round(peak_intensity/5)*5 as threshold, count(*) FROM peak WHERE ((338e-6 < peak_time AND peak_time < 360e-6) OR (370e-6 < peak_time)) GROUP BY threshold"' \
     using 1:2 with linespoints notitle
