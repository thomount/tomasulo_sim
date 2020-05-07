#include "config.h"
int Instr2Op(int instrType) {
	if (instrType == ADD_TYPE || instrType == SUB_TYPE || instrType == JUMP_TYPE) return ADD_OP;
	if (instrType == MULT_TYPE || instrType == DIV_TYPE) return MULT_OP;
	if (instrType == LD_TYPE) return LD_OP;
}