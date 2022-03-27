
all: calib_curve.pdf

calib_curve.tex: calib_curve.gnuplot
	@gnuplot -c $< $@

.PHONY: clean

%.pdf: %.tex ; pdflatex $<

clean:
	rm -f *.eps *.out *.aux *.log *-converted-to.pdf *.tex *~

veryclean: clean
	rm -f *.pdf
