all: 2020-02-07.pdf 2020-02-27.pdf

2020-02-07.tex: 2020-02-07_calib.org 2020-02-07.org regression.gnuplot
	gnuplot -c regression.gnuplot $@ 2020-02-07_calib.org 2020-02-07.org

2020-02-27.tex: 2020-02-27_calib.org 2020-02-07.org regression.gnuplot
	gnuplot -c regression.gnuplot $@ 2020-02-27_calib.org 2020-02-27.org

%.pdf: %.tex ; pdflatex $<
