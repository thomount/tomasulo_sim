#pragma once

#include "config.h"
#include "instr.h"
#include <vector>
#include <cstdio>
using namespace std;

class Sim {
protected:
	int *is = 0, *ex = 0, *wb = 0;		//一条指令三个阶段完成的周期
	vector<Instr> inst;
	int n;
	FILE * fi, * fo;
	unsigned int reg[32];
public:
	Sim() {
	}
	~Sim() {
		if (is != 0) delete is;
		if (ex != 0) delete ex;
		if (wb != 0) delete wb;
	}
	int calc(unsigned char type, unsigned int r1, unsigned int r2, unsigned int r3);
	void config(FILE * _fi, FILE * _fo);
	void init();
	void run();
	
};

//TODO Tomasolu 算法