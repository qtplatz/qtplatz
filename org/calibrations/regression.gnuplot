set terminal epslatex size 6in,6in color colortext standalone header \
"\\usepackage{graphicx}\n\\usepackage{amsmath}\n\\usepackage[version=3]{mhchem}\n\\usepackage{siunitx}"

set output ARG1
input = ARG2   # excluded
input_all = ARG3

set multiplot layout 2,1
set lmargin 10

print sprintf("************* input file: %s", input )

set datafile separator "|"

f(x) = a*x + b

fit f(x) input using (sqrt($4)):2 via a,b

set xrange [6:*]
set ylabel "Time (\\si{\\micro\\second})"
set xlabel "$\\sqrt{m}$"

plot input_all using (sqrt($4)):2 with points pt 6 ps 2 notitle \
     , input_all using (sqrt($4)):2:(sprintf('{\\tiny\\ce{%s}}',stringcolumn(3))) with labels offset 1,-0.5 notitle \
     , f(x) title sprintf("$t=%g\\cdot \\sqrt{m} + %g$", a, b)

set ylabel "Regression error (\\si{\\nano\\second})"
set logscale y

plot input_all using (sqrt($4)):(abs(f(sqrt($4))-($2))*1000) with impulses lc rgb "red" lw 4 axes x1y1 notitle
