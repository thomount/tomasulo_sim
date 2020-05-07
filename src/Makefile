all: main

main: main.o sim.o instr.o config.o tomasulo.o
	g++ main.o sim.o instr.o config.o tomasulo.o -o main -g -std=c++11

main.o: main.cpp sim.cpp sim.h instr.h config.h
	g++ main.cpp -c -o main.o -g -std=c++11

sim.o: sim.cpp sim.h instr.h config.h
	g++ sim.cpp -c -o sim.o -g -std=c++11

instr.o: instr.h config.h instr.cpp
	g++ instr.cpp -c -o instr.o -g -std=c++11

config.o: config.cpp config.h
	g++ config.cpp -c -o config.o -g -std=c++11

tomasulo.o: tomasulo.cpp sim.h instr.h config.h
	g++ tomasulo.cpp -c -o tomasulo.o -g -std=c++11
