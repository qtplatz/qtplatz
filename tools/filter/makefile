
all: filter
CC = clang++
CXX = clang++
CXXFLAGS = -std=c++11 -I/usr/local/boost-1_55/include -I../src/libs -DNDEBUG -O2

filter: filter.o ../../src/libs/adportable/sgfilter.o
	$(CC) -o filter filter.o ../../src/libs/adportable/sgfilter.o -lstdc++

run: filter
	./filter > data.txt
	gnuplot data.plt

#sgfilter.o: ../../src/libs/adportable/sgfilter.cpp
