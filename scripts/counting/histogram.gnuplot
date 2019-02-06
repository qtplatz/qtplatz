reset

set terminal epslatex size 8in,4in color colortext standalone header \
"\\usepackage{graphicx}\n\\usepackage{amsmath}\n\\usepackage[version=3]{mhchem}\n\\usepackage{siunitx}"
#set terminal x11
#set terminal postscript eps enhanced color size 8in,11.5in font 'Helvetica,24' linewidth 3
#set terminal epslatex size 10in,8in color colortext
#set terminal svg size 1280 720 fname 'Helvetica' fsize 16

set output ARG1
dbfile = ARG2

set datafile separator "|"

set multiplot layout 2,1
#------------------------------ frequency plot ----------------------------------
set title sprintf("Histogram")
set xlabel "Time($\\mu$s)"
set ylabel "Counts" # (^4He^{2+})"

plot '< sqlite3 ' . dbfile . ' \
"SELECT round(peak_time*1e9)/1000.0 as time,count(*) FROM trigger,peak WHERE id=idtrigger AND time > 338 GROUP BY time"' \
     using 1:($2) with impulse title "Time resolution: 1ns"

set notitle
set yrange [0:10]

plot '< sqlite3 ' . dbfile . ' \
"SELECT round(peak_time*1e9)/1000.0 as time,count(*) FROM trigger,peak WHERE id=idtrigger AND time > 338 GROUP BY time"' \
     using 1:2 with impulse title "Time resolution: 1ns"
