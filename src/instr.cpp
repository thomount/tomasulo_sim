#include "instr.h"
#include <cstdio>
#include <cstring>
int str2Type(char ch[20]) {
	if (ch[0] == 'L') 	//LD
		return LD_TYPE;
	if (ch[0] == 'A') 	//ADD
		return ADD_TYPE;
	if (ch[0] == 'S') 	//LD
		return SUB_TYPE;
	if (ch[0] == 'M') 	//ADD
		return MULT_TYPE;
	if (ch[0] == 'D') 	//LD
		return DIV_TYPE;
	if (ch[0] == 'J') 	//ADD
		return JUMP_TYPE;
}
unsigned int parseInt(char ch[20]) {	//立即数
	if (strlen(ch) > 1 && ch[1] == 'x') {	//十六
		unsigned int ret = 0;
		for (int i = 2; i < strlen(ch); i++) 
			ret = ret * 16 + ((ch[i]<='9')?(ch[i]-'0'):(ch[i]-'A'+10));
		return ret;
	} else {		
		unsigned int ret = 0;
		for (int i = 0; i < strlen(ch); i++) ret = ret * 10 + ch[i] - '0';
		return ret;
	}
}
unsigned int parseR(char ch[20]) {		//寄存器
	int i = 1, ret = 0;
	while (i < strlen(ch)) ret = ret * 10 + ch[i++]-'0';
	return ret;
}
Instr parse(char s[100]) {
	//根据字符串解析出编译命令
	char op[4][20];
	memset(op, 0, sizeof(op));
	int st = 0, w = 0, i = 0, l = strlen(s);
	while (st < l) {
		if (s[st] != ',') op[w][i++] = s[st++]; else {
			w++; i = 0; st++;
		}
	}
	int opt = str2Type(op[0]);
	Instr ret(opt,0,0,0);
	if (opt == LD_TYPE) {
		ret = Instr(opt, parseR(op[1]), parseInt(op[2]));
	}
	if (opt == ADD_TYPE || opt == SUB_TYPE || opt == MULT_TYPE || opt == DIV_TYPE) {
		ret = Instr(opt, parseR(op[1]), parseR(op[2]), parseR(op[3]));
	}
	if (opt == JUMP_TYPE) {
		ret = Instr(opt, parseInt(op[1]), parseR(op[2]), parseInt(op[3]));
	}

	//printf("%d\t%d\t%d\t%d\n", ret.type, ret.d1, ret.d2, ret.d3);
	return ret;
}