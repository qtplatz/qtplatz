
all: pkarea-simu.pdf

pkarea-simu.tex: profile.gnuplot
	gnuplot -c $< $@

.SUFFIXES: .tex .pdf

%.pdf: %.tex
	pdflatex $<
