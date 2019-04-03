#set terminal postscript eps enhanced color size 11.5in,8.0in font 'Helvetica,24' linewidth 3
set terminal epslatex size 9in,18in color colortext standalone header \
"\\usepackage{graphicx}\n\\usepackage{amsmath}\n\\usepackage[version=3]{mhchem}"

set output ARG1

N = ARGC-1

array ARGV[ N ]
array FILE[ N ]
array TITLE[ N ]
array max_y[ N ]
array min_x[ N ]
array max_x[ N ]
array AVG[ N ]
array AVG10[ N ]
array AVG50[ N ]
array AVG75[ N ]
array Vmcp[ N ]

do for [i=1 : N] {
  eval sprintf("ARGV[%d] = ARG%d", i, i+1)
  if ( words( ARGV[i] ) == 3 ) {
    FILE[i] = word( ARGV[i], 1 )
    TITLE[i] = word( ARGV[i], 2 )
    Vmcp[i] = word(  ARGV[i], 3 ) + 0
  }
}

do for [i=1 : N] { print sprintf("FILE: %s;   TITLE: %s", FILE[i], TITLE[i]) }

tof = 99.91e-6
tof_width = 2e-9
tof_lower = tof - tof_width / 2
tof_upper = tof + tof_width / 2
step = 5

SQL = sprintf( "'\
SELECT *,COUNT(*) AS COUNTS FROM (SELECT MIN(peak_time),ROUND(peak_intensity/%d)*%d AS Threshold \
FROM trigger,peak WHERE id=idTrigger AND peak_time > %.14g AND peak_time < %.14g GROUP BY id) \
GROUP BY Threshold'", step, step, tof_lower, tof_upper )

SQL2 = "'\
SELECT sum(-Threshold*COUNTS)/sum(COUNTS) FROM \
(SELECT *,COUNT(*) AS COUNTS FROM \
(SELECT MIN(peak_time),ROUND(peak_intensity/%d)*%d AS Threshold FROM trigger,peak WHERE id=idTrigger \
AND peak_time > %.14g AND peak_time < %.14g GROUP BY id) \
GROUP BY Threshold ) WHERE COUNTS > %.14g'"

set multiplot layout (N > 2) ? N + 1 : N,1

set ylabel "Frequency(counts)"
set xlabel "Peak height(-mV)"
set datafile separator "|"

do for [i=1:N] {
  stats '< sqlite3 '. FILE[i] . " " . SQL using (-$2):3 name 'st'.i nooutput
  eval sprintf( "max_y[%d] = st%d_max_y", i, i )
  eval sprintf( "min_x[%d] = st%d_min_x", i, i )
  eval sprintf( "max_x[%d] = st%d_max_x", i, i )

  AVG[i] = system( 'sqlite3 '. FILE[i] . " " .   sprintf( SQL2, step, step, tof_lower, tof_upper, 0 ) ) + 0
  AVG10[i] = system( 'sqlite3 '. FILE[i] . " " . sprintf( SQL2, step, step, tof_lower, tof_upper, max_y[i]*0.1 ) ) + 0
  AVG50[i] = system( 'sqlite3 '. FILE[i] . " " . sprintf( SQL2, step, step, tof_lower, tof_upper, max_y[i]*0.5 ) ) + 0
  AVG75[i] = system( 'sqlite3 '. FILE[i] . " " . sprintf( SQL2, step, step, tof_lower, tof_upper, max_y[i]*0.75 ) ) + 0
}

set xrange [ 0 : 800 ]

do for [i=1:N] {

set arrow 1 from AVG[ i ], graph 0 to AVG[ i ], graph 1 nohead filled back lw 3 lc rgb 'blue'
set arrow 2 from AVG10[ i ], graph 0 to AVG10[ i ], graph 1 nohead filled back lw 3 lc rgb 'green'
set arrow 3 from AVG50[ i ], graph 0 to AVG50[ i ], graph 1 nohead filled back lw 3 lc rgb 'red'
#set arrow 4 from AVG75[ i ], graph 0 to AVG75[ i ], graph 1 nohead filled back lw 3 lc rgb 'orange'
set label 1 sprintf("$\\overline{m}=%.1fmV$",          AVG[i]) at graph 0.8, 0.8
set label 2 sprintf("$\\overline{m}=%.1fmV(10\\%)$", AVG10[i]) at graph 0.8, 0.7
set label 3 sprintf("$\\overline{m}=%.1fmV$(50\\%)", AVG50[i]) at graph 0.8, 0.6
#set label 4 sprintf("$\\overline{m}=%.1fmV$(75\\%)", AVG75[i]) at graph 0.8, 0.5

legend = sprintf( "%s (TOF:$%.3f\\mu s\\pm %.1fns$)", TITLE[i], tof * 1.0e6, tof_width * 1.0e9 )

plot '< sqlite3 '. FILE[i] . " " . SQL using (-$2):3 with linespoints pt 6 ps 2 axis x1y1 title legend \
     , max_y[ i ] * 1.0 with lines dashtype 2 lc rgb 'gray' notitle \
     , max_y[ i ] * 0.1 with lines dashtype 2 lc rgb 'gray' notitle \
     , max_y[ i ] * 0.5 with lines dashtype 2 lc rgb 'red' notitle
}

unset label 1
unset label 2
unset label 3
#unset label 4
unset arrow 1
unset arrow 2
unset arrow 3
#unset arrow 4

unset xrange
set key left top

f(x) = a + b*x + c*x*x

if ( N > 2 ) {

  set ylabel "Average peak height(mV)"
  set xlabel "MCP Voltage (V)"

  fit f(x) AVG using (Vmcp[$0+1]):(AVG[$0+1]) via a,b,c

  plot AVG using (Vmcp[$0+1]):(AVG[$0+1]) with linespoints pt 5 ps 2.5 title "Average(all)" \
     , AVG10 using (Vmcp[$0+1]):(AVG10[$0+1]) with linespoints pt 6 ps 2.5 title "Average(10\\%)" \
     , AVG50 using (Vmcp[$0+1]):(AVG50[$0+1]) with linespoints pt 7 ps 2.5 title "Average(50\\%)" \
     , AVG75 using (Vmcp[$0+1]):(AVG75[$0+1]) with linespoints pt 7 ps 2.5 title "Average(75\\%)"

  do for [i=1:N] {
    print sprintf("%d: %g, %g, %g, %g", i, AVG[i], AVG10[i], AVG50[i], AVG75[i] )
  }
}
