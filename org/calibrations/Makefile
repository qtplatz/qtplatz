all: 2020-02-07.pdf

2020-02-07.tex: 2020-02-07.org regression.gnuplot
	gnuplot -c regression.gnuplot $@ $<

%.pdf: %.tex ; pdflatex $<
