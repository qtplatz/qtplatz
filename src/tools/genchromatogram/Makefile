EXECDIR=~/src/build-Darwin-arm64/qtplatz-qt6.7.release/bin/qtplatz.app/Contents/MacOS

all: ex01.pdf ex02.pdf ex03.pdf

ex01.tex: ex01.gnuplot ${EXECDIR}/genChromatogram
	@gnuplot -c $< $@ "${EXECDIR}/genChromatogram"

ex02.tex: ex02.gnuplot ${EXECDIR}/genChromatogram
	@gnuplot -c $< $@ "${EXECDIR}/genChromatogram"

ex03.tex: ex03.gnuplot ${EXECDIR}/genChromatogram
	@gnuplot -c $< $@ "${EXECDIR}/genChromatogram"

clean:
	rm -f *.eps *.out *.aux *.log *-eps-converted-to.pdf *.tex *~

veryclean: clean
	rm -f *.pdf

%.pdf: %.tex ; pdflatex $<

%.out: %.mop ; mopac $<
