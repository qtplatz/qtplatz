#set terminal postscript eps enhanced color size 11.5in,8.0in font 'Helvetica,24' linewidth 3
set terminal epslatex size 5in,4in color colortext standalone header \
    "\\usepackage{graphicx}\n\\usepackage{amsmath}\n\\usepackage[version=3]{mhchem}\n\\usepackage{siunitx}"

set output ARG1

# negative
f1(x) = (-8.47724598e-02 + 5.89119360e+05 * x + -2.53670299e+06 * x**2 + 1.96715948e+10 * x**3)**2
g1(x) = (-8.47724598e-02 + 5.89119360e+05 * x)**2

f2(x) = (-8.28598134e-02 + 5.89312567e+05 * x + -9.66214385e+05 * x**2 + 1.01645327e+10 * x**3)**2
g2(x) = (-8.28598134e-02 + 5.89312567e+05 * x)**2

set ylabel "Error $m/z$"
set xlabel "TOF \\si{\\micro\\second}"

set xrange [15e-6:70e-6]

set format x "%.0s"
#set format y "%.0t%c"
#set format y "$%.0s*10^{%T}$"

plot f1(x)-g1(x) with lines linestyle 1 lw 2 title "negative ion mode"\
     , f2(x)-g2(x) with lines linestyle 2 lw 2 title "positive ion mode"
