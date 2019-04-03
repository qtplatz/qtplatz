#set terminal postscript eps enhanced color size 11.5in,8.0in font 'Helvetica,24' linewidth 3
set terminal epslatex size 8in,16in color colortext standalone header \
"\\usepackage{graphicx}\n\\usepackage{amsmath}\n\\usepackage[version=3]{mhchem}\\usepackage{siunitx}"

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

set multiplot layout 4,1

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

unset xrange
#set key left top

f(x) = a + b*x + c*x*x + d*x*x*x

set ylabel "Peak height(mV)"
set xlabel "MCP Voltage (V)"
set xrange [1400:2100]

fit f(x) AVG using (Vmcp[$0+1]):(AVG[$0+1]) via a,b,c,d
plot AVG using (Vmcp[$0+1]):(AVG[$0+1]) with linespoints pt 5 ps 2.5 title "Average(all)" \
     ,f(x) title sprintf( "$Peak(mV) = \\num{%g} + \\num{%g}\\cdot x + \\num{%g}\\cdot x^2 + \\num{%g}\\cdot x^3$", a, b, c, d ) with lines lw 5

fit f(x) AVG10 using (Vmcp[$0+1]):(AVG10[$0+1]) via a,b,c,d
plot AVG10 using (Vmcp[$0+1]):(AVG10[$0+1]) with linespoints pt 5 ps 2.5 title "Average(10\\%)" \
     ,f(x) title sprintf( "$Peak(mV) = \\num{%g} + \\num{%g}\\cdot x + \\num{%g}\\cdot x^2 + \\num{%g}\\cdot x^3$", a, b, c, d ) with lines lw 5

fit f(x) AVG50 using (Vmcp[$0+1]):(AVG50[$0+1]) via a,b,c,d
plot AVG50 using (Vmcp[$0+1]):(AVG50[$0+1]) with linespoints pt 5 ps 2.5 title "Average(50\\%)" \
     ,f(x) title sprintf( "$Peak(mV) = \\num{%g} + \\num{%g}\\cdot x + \\num{%g}\\cdot x^2 + \\num{%g}\\cdot x^3$", a, b, c, d ) with lines lw 5

fit f(x) AVG75 using (Vmcp[$0+1]):(AVG75[$0+1]) via a,b,c,d
plot AVG75 using (Vmcp[$0+1]):(AVG75[$0+1]) with linespoints pt 5 ps 2.5 title "Average(75\\%)" \
     ,f(x) title sprintf( "$Peak(mV) = \\num{%g} + \\num{%g}\\cdot x + \\num{%g}\\cdot x^2 + \\num{%g}\\cdot x^3$", a, b, c, d ) with lines lw 5
