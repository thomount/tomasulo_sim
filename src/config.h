#pragma once

//空间大小
const int ADD_N = 3;
const int MULT_N = 2;
const int LOAD_N = 2;
const int ADD_S = 6;
const int MULT_S = 3;
const int LOAD_S = 3;

// 指令类型
const int ADD_TYPE = 0;
const int SUB_TYPE = 1;
const int MULT_TYPE = 2;
const int DIV_TYPE = 3;
const int LD_TYPE = 4;
const int JUMP_TYPE = 5;
const int CALC_T[6] = {3,3,4,4,3,1};

//运算器类型
const int ADD_OP = 0;
const int MULT_OP = 1;
const int LD_OP = 2;
const int FU_N[3] = {3,2,2};
const int RS_N[3] = {6,3,3};
int Instr2Op(int);		//根据指令类型返回其需要的运算单元类型