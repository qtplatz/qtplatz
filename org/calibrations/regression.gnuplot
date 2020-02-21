set terminal epslatex size 6in,4in color colortext standalone header \
"\\usepackage{graphicx}\n\\usepackage{amsmath}\n\\usepackage[version=3]{mhchem}\n\\usepackage{siunitx}"

set output ARG1
input = ARG2

print sprintf("************* input file: %s", input )

set datafile separator "|"

f(x) = a*x + b
fit f(x) input using (sqrt($4)):2 via a,b
set y2tics
set xrange [6:*]
#set y2range [ -0.1:+0.1]
set xlabel "Time (\\si{\\micro\\second})"
set ylabel "$\\sqrt{m}$"
set y2label "Error \\si{\\milli\\dalton}"

#plot input using (sqrt($4)):2 with linespoints notitle
plot input using (sqrt($4)):2 with linespoints pt 6 ps 2 notitle \
     , input using (sqrt($4)):2:(sprintf('{\\tiny\\ce{%s}}',stringcolumn(3))) with labels offset 1,-0.5 notitle \
     , f(x) title sprintf("$\\sqrt{m}=%g\\cdot t + %g$", a, b) \
     , input using (sqrt($4)):(((f(sqrt($4))-($2))*(f(sqrt($4))-($2)))*1000) with impulses lc rgb "red" lw 4 axes x1y2 notitle \
     , 0 lc rgb "red" axes x1y2 notitle
