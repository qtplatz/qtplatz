
all: filter
#CC = clang++
CXXFLAGS = -std=c++11 -I/usr/local/boost-1_55/include -DNDEBUG -O2

filter: filter.o
	$(CC) -o filter filter.o -lstdc++
