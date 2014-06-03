CXX=c++
CC=cc
CFLAGS += -O2 -Wall -W
CXXFLAGS += -O2 -Wall -W

TARGETS=binaryC binaryCpp binaryCpp2

all: binaryC.o binaryCpp.o binaryCpp2.o
	$(CC) $(CFLAGS) -o binaryC binaryC.o -lm
	$(CXX) $(CXXFLAGS) -o binaryCpp binaryCpp.o
	$(CXX) $(CXXFLAGS) -o binaryCpp2 binaryCpp2.o

clean:
	rm -f *.o $(TARGETS)
