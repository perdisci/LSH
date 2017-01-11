CC=gcc
CPP=g++
OPSCPP=-std=c++11 -O3
OPSCC=-O3
INC=-I/Users/perdisci
LIB=-L/Users/perdisci/boost/boostlibs
BOOST=-lboost_system -lboost_filesystem -lboost_program_options

lsh_example: lsh_example.o minhash.o xxhash.o lsh.o
	$(CPP) $(OPSCPP) -o lsh_example lsh_example.o minhash.o xxhash.o lsh.o $(LIB) $(BOOST)

lsh_example.o:
	$(CPP) $(OPSCPP) -c lsh_example.cpp $(INC)

minhash.o: minhash.cpp minhash.hpp
	$(CPP) $(OPSCPP) -c minhash.cpp $(INC)

lsh.o: lsh.cpp lsh.hpp
	$(CPP) $(OPSCPP) -c lsh.cpp $(INC)

xxhash.o: ./xxHash/xxhash.c ./xxHash/xxhash.h
	$(CC) $(OPSCC) -c ./xxHash/xxhash.c 

clean:
	rm -f lsh_example lsh_example.o minhash.o xxhash.o lsh.o
