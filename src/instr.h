#pragma once
#include "config.h"

struct Instr{
	unsigned char type;
	unsigned int d1, d2, d3;
	Instr(unsigned char _type, unsigned int _d1, unsigned int _d2, unsigned int _d3=0): type(_type), d1(_d1), d2(_d2), d3(_d3) {}
};

Instr parse(char s[100]);