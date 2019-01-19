
all: histogram.pdf frequency.pdf

histogram.tex: ~/data/imaoka/peaks.db histogram.gnuplot
	gnuplot -c histogram.gnuplot $@ $<

frequency.tex: ~/data/imaoka/peaks.db frequency.gnuplot
	gnuplot -c frequency.gnuplot $@ $<

%.pdf: %.tex ; pdflatex $<
