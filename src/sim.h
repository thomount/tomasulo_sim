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
	virtual void init();
	virtual void run();
	
};

class Func {
public:
	int op, left, tar;
	bool busy = false;
	unsigned int a, b;
	Func() {}
	bool tick(unsigned int & ret);
	void set(int, unsigned int, unsigned int, int);
};

struct station{	//
	int busy;	//0 Null 1 等待 2 就绪 3 运行中 4 写回中
	int op;		//操作
	int tar;	//进入运算区后负责的运算器
	unsigned int v1, v2;
	int q1, q2, ret;
	int cp;
	int clock;
	station():busy(0) {}
};

struct queue {
	int Q[10];
	station *C[10];
	int l, r, n;
	queue():n(0) {}
	queue(int _size) {
		n = _size;
		l = 0, r = 0;
	}
	int size();
	void add(int, station *);
	int pop();
};
//Tomasolu 算法

struct WriteEvent {
	int reg, fu=-1;
	unsigned int val;
	WriteEvent() {
	}
};
class Tomasulo: public Sim {
protected:
	//空间：RF(寄存器对应FU)，rs(保留站)，lb(读取缓存)
	int rf[32];		//-1表示无
	station RS[ADD_S+MULT_S+LOAD_S];

	int use[3], que[3];				//三种运算单元使用情况,保留站使用情况
	queue pending[3];
	const int RS_st[3] = {0,ADD_S, ADD_S+MULT_S};
	const int RS_en[3] = {ADD_S, ADD_S+MULT_S, ADD_S+MULT_S+LOAD_S};
	const int FU_st[3] = {0,ADD_N, ADD_N+MULT_N};
	const int FU_en[3] = {ADD_N, ADD_N+MULT_N, ADD_N+MULT_N+LOAD_N};
	Func f[ADD_N+MULT_N+LOAD_N];
	int cp, cp_next, clock;
	bool block;
	WriteEvent WB[ADD_N+MULT_N+LOAD_N];
	int WB_top;
public:
	Tomasulo() {
	}
	~Tomasulo() {

	}
	int calc(unsigned char type, unsigned int r1, unsigned int r2, unsigned int r3);
	//void config(FILE * _fi, FILE * _fo);
	void init();
	void run();
	void emit(Instr, int, int, int);
	void databus(int fu, unsigned int val);
	void add_pending(int);
	void check_pending();
	void send(int index);
	void print(int flag = -1);
	void add_writer(int, int, int);
	void clean_writer();
	void show(int flag);
	void show_regs(int);
	void writer_preclean(int, int);
};