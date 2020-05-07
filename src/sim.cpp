#include "sim.h"
#include <cstdio>
#include <cstring>

void Sim::config(FILE * _fi, FILE * _fo) {
	fi = _fi;
	fo = _fo;
	if (is != 0) delete is;
	if (ex != 0) delete ex;
	if (wb != 0) delete wb;
	inst.clear();
	char ins[100];
	while(fscanf(fi, "%s", ins) >= 0) {
		inst.push_back(parse(ins));
	}
	n = inst.size();
	is = new int[n];
	ex = new int[n];
	wb = new int[n];
	memset(is, 0, (n<<2));
	memset(ex, 0, (n<<2));
	memset(wb, 0, (n<<2));
}

void Sim::init() {
	memset(reg, 0, sizeof(reg));
}
void Sim::run() {
	//fprintf(fo, "%d\n", n);
	//TODO excute instrs
	for (int i = 0, c = 1, j = 1; i < n; i+=j, c++) {
		//fprintf(fo, "inst %d: %d\t %d\t %d\t %d\n", i, inst[i].type, inst[i].d1, inst[i].d2, inst[i].d3);
		//printf("reg:\t");
		//for (int k = 1; k <= 4; k++) printf("%d\t", reg[k]);
		//printf("\n");
		if (!is[i]) {
			is[i] = c;
			ex[i] = (c += (inst[i].type==DIV_TYPE && reg[inst[i].d3] == 0)? 1: CALC_T[inst[i].type]);
			wb[i] = (++c);
		}
		//fprintf(fo, "%d %d %d %d\n", i, is[i], ex[i], wb[i]);
		//for (int k = 0; k < 5; k++) fprintf(fo, "%d ", reg[k]); fprintf(fo, "\n");
		j = calc(inst[i].type, inst[i].d1, inst[i].d2, inst[i].d3);
		//printf("ret = %d\n" , j);
		
	}
	for (int i = 0; i < n; i++) fprintf(fo, "%d %d %d %d\n", i, is[i], ex[i], wb[i]);
	fprintf(fo, "\nRegister usage:\n");
	for (int i = 0; i < 32; i++) fprintf(fo, "Reg %3d:%10d\n", i, reg[i]);
}

int Sim::calc(unsigned char type, unsigned int r1, unsigned int r2, unsigned int r3) {
	if (type == JUMP_TYPE) {
		if (reg[r2] == r1) return r3;
	}
	if (type == ADD_TYPE) reg[r1] = reg[r2]+reg[r3];
	if (type == SUB_TYPE) reg[r1] = reg[r2]-reg[r3];
	if (type == MULT_TYPE) reg[r1] = reg[r2]*reg[r3];
	if (type == DIV_TYPE) reg[r1] = (reg[r3] != 0)?(reg[r2]/reg[r3]): reg[r2];
	if (type == LD_TYPE) reg[r1] = r2;
	return 1;
}