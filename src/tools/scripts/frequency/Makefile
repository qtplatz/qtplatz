HOST=$(shell hostname)
UNAME := $(shell uname)
DATAROOT = ~/data/mouse/kobayashi/2019-03-29
DATAFILE1 = Ar_20laps_5kV_1500V_10mV_0001.adfs
DATAFILE2 = Ar_20laps_5kV_1600V_10mV_0001.adfs
DATAFILE3 = Ar_20laps_5kV_1700V_10mV_0001.adfs
DATAFILE4 = Ar_20laps_5kV_1800V_0001.adfs
DATAFILE5 = Ar_20laps_5kV_1900V_0001.adfs
DATAFILE6 = Ar_20laps_5kV_2000V_0001.adfs

#all: frequency1.pdf frequency2.pdf frequency3.pdf frequency4.pdf frequency5.pdf frequency6.pdf frequency-all.pdf tof-intensity.pdf
all: frequency_5mV.pdf

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

tof-intensity.tex: ${DATAFILE} tof-intensity.gnuplot
	gnuplot -c tof-intensity.gnuplot $@ $<

%.pdf: %.tex ; pdflatex $<

clean:
	rm -f *.eps *.out *.aux *.log *.pdf *.tex *~
