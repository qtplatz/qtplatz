HOST=$(shell hostname)
UNAME := $(shell uname)
DATAROOT = ~/data/mouse/kobayashi/2019-03-29
DATAFILE1 = Ar_20laps_5kV_1500V_10mV_0001.adfs
DATAFILE2 = Ar_20laps_5kV_1600V_10mV_0001.adfs
DATAFILE3 = Ar_20laps_5kV_1700V_10mV_0001.adfs
DATAFILE4 = Ar_20laps_5kV_1800V_0001.adfs
DATAFILE5 = Ar_20laps_5kV_1900V_0001.adfs
DATAFILE6 = Ar_20laps_5kV_2000V_0001.adfs

#all: frequency1.pdf frequency2.pdf frequency3.pdf frequency4.pdf frequency5.pdf frequency6.pdf frequency-all.pdf
all: frequency_m.pdf

frequency1.tex: ${DATAROOT}/${DATAFILE1} frequency.gnuplot Makefile
	gnuplot -c frequency.gnuplot $@ $< "Argon, 20laps, 5kV, 1.5kV"

frequency2.tex: ${DATAROOT}/${DATAFILE2} frequency.gnuplot Makefile
	gnuplot -c frequency.gnuplot $@ $< "Argon, 20laps, 5kV, 1.6kV"

frequency3.tex: ${DATAROOT}/${DATAFILE3} frequency.gnuplot Makefile
	gnuplot -c frequency.gnuplot $@ $< "Argon, 20laps, 5kV, 1.7kV"

frequency4.tex: ${DATAROOT}/${DATAFILE4} frequency.gnuplot Makefile
	gnuplot -c frequency.gnuplot $@ $< "Argon, 20laps, 5kV, 1.8kV"

frequency5.tex: ${DATAROOT}/${DATAFILE5} frequency.gnuplot Makefile
	gnuplot -c frequency.gnuplot $@ $< "Argon, 20laps, 5kV, 1.9kV"

frequency6.tex: ${DATAROOT}/${DATAFILE6} frequency.gnuplot Makefile
	gnuplot -c frequency.gnuplot $@ $< "Argon, 20laps, 5kV, 2.0kV"

frequency_5mV.tex: ${DATAROOT}/Ar_20laps_5kV_1500V.adfs frequency.gnuplot Makefile
	gnuplot -c frequency.gnuplot $@ $< "Argon, 20laps, 5kV, 2.0kV 5mV"

frequency_m.tex: ${DATAROOT}/Ar_20laps_5kV_1500V.adfs frequency_m.gnuplot Makefile
	@gnuplot -c frequency_m.gnuplot $@ \
	"'${DATAROOT}/${DATAFILE1}' 'Argon, 20laps, 5kV, 1.5kV' '1500'" \
	"'${DATAROOT}/${DATAFILE2}' 'Argon, 20laps, 5kV, 1.6kV' '1600'" \
	"'${DATAROOT}/${DATAFILE3}' 'Argon, 20laps, 5kV, 1.7kV' '1700'" \
	"'${DATAROOT}/${DATAFILE4}' 'Argon, 20laps, 5kV, 1.8kV' '1800'" \
	"'${DATAROOT}/${DATAFILE5}' 'Argon, 20laps, 5kV, 1.9kV' '1900'" \
	"'${DATAROOT}/${DATAFILE6}' 'Argon, 20laps, 5kV, 2.0kV' '2000'"

%.pdf: %.tex ; pdflatex $<

clean:
	rm -f *.eps *.out *.aux *.log *.pdf *.tex *~
