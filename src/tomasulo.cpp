#include "sim.h"
#include <cstring>
#include <cstdio>

static const char name[6][5] = {"ADD", "SUB", "MULT", "DIV", "LOAD", "JUMP"};

void Func::set(int _op, unsigned int _a, unsigned int _b, int _tar) {
	op = _op; a = _a; b = _b; tar = _tar;
	//printf("%s %d %d\n", name[op], a, b);
	left = CALC_T[op];
	if (op == DIV_TYPE && b == 0) left = 1;
	busy = true;
}
bool Func::tick(unsigned int & ret) {
	if (!busy) return false;
	left --;
	if (left == 0) {
		if (op == ADD_TYPE) ret = a+b;
		if (op == SUB_TYPE) ret = a-b;
		if (op == MULT_TYPE) ret = a*b;
		if (op == DIV_TYPE) ret = a/((b)?b:1);
		if (op == LD_TYPE) ret = b;
		if (op == JUMP_TYPE) ret = (a == b)?1:0;
		busy = false;
		return true;
	}
	return false;
}

//tomasulo算法功能
int Tomasulo::calc(unsigned char type, unsigned int r1, unsigned int r2, unsigned int r3) {}
//void Tomasulo::config(FILE * _fi, FILE * _fo) {}
void Tomasulo::init() {
	memset(reg, 0, sizeof(reg));
	memset(rf, -1, sizeof(rf));
	memset(use, 0, sizeof(use));
	memset(que, 0, sizeof(que));
	//补全init
	for (int i = 0; i < ADD_N+MULT_N+LOAD_N; i++) f[i].busy = false;
	for (int i = 0; i < ADD_S+MULT_S+LOAD_S; i++) RS[i].busy = 0, RS[i].vir = false;
	for (int i = 0; i < ADD_N+MULT_N+LOAD_N; i++) WB[i].fu = -1;
	WB_top = 0;
	pending[0] = queue(ADD_S+2);
	pending[1] = queue(MULT_S+2);
	pending[2] = queue(LOAD_S+2);
	//printf("%d %d %d\n", pending[0].Q, pending[1].Q, pending[2].Q);
	cp = 0, clock = 0;
	block = false;
	vir = false;
	predict = new bool[n];
	for (int i = 0; i < n; i++) predict[i] = true;
	acc = ptot = 0;
}

void Tomasulo::emit(Instr ins, int st, int en, int cp) {	//指令进入保留站
	for (int i = st; i < en; i++) {
		if (RS[i].busy == 0) {
			//printf("emiting %d: %s\t%d\t%d\t%d\t   into   %d\n", cp, name[ins.type], ins.d1, ins.d2, ins.d3, i);
			if (ins.type <= DIV_TYPE)
				writer_preclean(ins.d1, i);

			RS[i].op = ins.type;
			RS[i].busy = 1;
			RS[i].cp = cp;
			RS[i].tar = -1;
			RS[i].v1 = 0;
			RS[i].v2 = 0;
			RS[i].vir = vir;
			if (ins.type <= DIV_TYPE) {
				if (!vir) {
					RS[i].q1 = rf[ins.d2];
					if (rf[ins.d2] == -1) RS[i].v1 = reg[ins.d2];
					RS[i].q2 = rf[ins.d3];
					if (rf[ins.d3] == -1) RS[i].v2 = reg[ins.d3];
				} else {
					RS[i].q1 = vrf[ins.d2];
					if (vrf[ins.d2] == -1) RS[i].v1 = vreg[ins.d2];
					RS[i].q2 = vrf[ins.d3];
					if (vrf[ins.d3] == -1) RS[i].v2 = vreg[ins.d3];
				}
				RS[i].ret = ins.d1;
				//printf("load cp = %d: %s R%d R%d R%d -> R%d(%d)\n", RS[i].cp, name[RS[i].op], ins.d1, ins.d2, ins.d3, ins.d1, i);
				if (vir) vrf[RS[i].ret] = i; else rf[RS[i].ret] = i;
			}
			if (ins.type == LD_TYPE) {
				RS[i].q1 = -1; RS[i].v1 = 0;
				RS[i].q2 = -1; RS[i].v2 = ins.d2;
				RS[i].ret = ins.d1;
				if (vir) vrf[RS[i].ret] = i; else rf[RS[i].ret] = i;
			}
			if (ins.type == JUMP_TYPE) {
				if (vir)  {
					printf("error: emit JUMP when predicting\n");
				}
				RS[i].q1 = -1; RS[i].v1 = ins.d1;
				RS[i].q2 = rf[ins.d2];
				if (rf[ins.d2] == -1) RS[i].v2 = reg[ins.d2];
				RS[i].ret = ins.d3;	
				//block = true;							//中断防止后续指令进入
				//TODO+ 若当前非虚拟状态，则建立分支空间，并把当前RS、reg拷贝，并将状态改为虚拟
				//		若已经是虚拟状态，则阻塞直到虚拟状态解除
				if (predict_flag == false) block = true; else {
					if (vir == false) {
						vir = true;
						block = false;
						cp_next = cp + ((predict[cp])?ins.d3:1);
						branch = cp;

						memcpy(vrf, rf, sizeof(rf));
						memcpy(vreg, reg, sizeof(reg));
						//printf("predicting on cp=%d with %d\n", cp, predict[cp]);
					} else {
						block = true;
					}
				}
			} else cp_next = cp+1;
			//if (RS[i].q1 == -1 && RS[i].q2 == -1) {		统一就绪
			//	RS[i].busy = 2;
			//	add_pending(i);
			//}
			break;
		}
	}
}

//pending队列维护
void Tomasulo::add_pending(int index) {				//保留站中就绪的指令加入排队
	//printf("ready cp = %d\n", RS[index].cp);
	if(RS[index].busy != 1) return;
	RS[index].busy = 2;
	RS[index].clock = clock;
	int type = Instr2Op(RS[index].op);
	pending[type].add(index, &RS[index]);
//	printf("add %d\n", type);
//	printf("%d %d %d\n", pending[0].size(), pending[1].size(), pending[2].size());
}
void Tomasulo::check_pending() {					//从队列中将就绪排队指令送入运算器
	//printf("check pending + \n");
//	printf("%d %d %d\n", pending[0].size(), pending[1].size(), pending[2].size());
	for (int i = 0; i < 3; i++) {
		while (use[i] < FU_N[i] && pending[i].size())	//可以进入
			send(pending[i].pop());
//		printf("%d %d %d\n", pending[0].size(), pending[1].size(), pending[2].size());
	}
	//printf("check pending - \n");
}

//将一条指令从队列送入运算器
void Tomasulo::send(int index) {
	if (RS[index].busy != 2) return;
//	printf("send %d\n", index);
	int type = Instr2Op(RS[index].op);
	use[type] ++;
	int tar = -1;
	for (int i = FU_st[type]; i < FU_en[type]; i++)
		if (f[i].busy == false) {tar = i; break;}
	RS[index].tar = tar;
	RS[index].busy = 3;
	//printf("cp = %d: ", RS[index].cp);
	f[tar].set(RS[index].op, RS[index].v1, RS[index].v2, index);
}


void Tomasulo::run() {
	bool BUSY = true;
	while (cp < n || BUSY) {
		clock ++;
		check_pending();			//就绪开始运行
		if (cp < n) {
			Instr ins = inst[cp];		//取指令
			int type = Instr2Op(ins.type);
			//printf("condition : %d %d %d\n", vir, type==JUMP_TYPE, (!vir || type != JUMP_TYPE));
			if (!block && (!vir || ins.type != JUMP_TYPE) && que[type] < RS_N[type]) {		//未阻塞且保留站有空间
				if (!is[cp]) is[cp] = clock;
				que[type] ++;
				emit(ins, RS_st[type], RS_en[type], cp);
			}
			
		}
		print(-1);
		clean_writer();
		//在写回后判断就绪
		for (int i = 0; i < ADD_S+MULT_S+LOAD_S; i++) {
			if (RS[i].q1 == -1 && RS[i].q2 == -1 && RS[i].busy == 1) add_pending(i);
		}
		for (int i = 0; i < ADD_N+MULT_N+LOAD_N; i++) {
			if (f[i].busy == false) continue;
			unsigned int ret = 0;
			if (f[i].tick(ret)) {		//运行结束
				databus(f[i].tar, ret);
				use[Instr2Op(f[i].op)]--;		//单元使用情况更新
			}
		}
		print(-1);
		//show_regs(12);
		cp = cp_next;
		BUSY = que[0] || que[1] || que[2] || use[0] || use[1] || use[2] || WB_top;
	}
	//printf("over\n");
	//show(0);
	show(1);
	//printf("print over\n");
}
void Tomasulo::show(int flag){
	if (!flag) {
		for (int i = 0; i < n; i++)
			printf("%d %d %d\n", is[i], ex[i], wb[i]);
		printf("\nRegister:\n");
		for (int i = 0; i < 32; i++) printf("Reg %3d: %10d\n", i, reg[i]);
		printf("Total clocks = %d\n", clock);
		if (predict_flag) printf("prediction accuracy = %.3f\n", 100.0*acc/ptot);
	} else {
//		printf("start printing\n");
//		printf("%d\n", f);
		for (int i = 0; i < n; i++) {
//			printf("line %d\n", i);
//			printf("%d %d %d\n", is[i], ex[i], wb[i]);
			fprintf(fo, "%d %d %d\n", is[i], ex[i], wb[i]);
		}
		if (printConfig == 1) return;
		fprintf(fo, "\nRegister:\n");
		for (int i = 0; i < 32; i++) {
//			printf("Reg %3d: %10d\n", i, reg[i]);
			fprintf(fo, "Reg %3d: %10d\n", i, reg[i]);
		}
		fprintf(fo, "Total clocks = %d\n", clock);
		if (predict_flag && ptot > 0) fprintf(fo, "prediction accuracy = %.3f\n", 100.0*acc/ptot);
	}
}
 
void Tomasulo::databus(int fu, unsigned int val) {
	if (RS[fu].busy != 3) return;		//指令已经失效，不广播
	//指令执行结束
	//printf("Excute over : %d %d %d %d\n", fu, val, RS[fu].cp, RS[fu].ret);
	if (!ex[RS[fu].cp]) ex[RS[fu].cp] = clock;
	if (RS[fu].op != JUMP_TYPE) {
		if (!wb[RS[fu].cp]) wb[RS[fu].cp] = clock+1;
		//printf("%d %d %d\n", is[RS[fu].cp], ex[RS[fu].cp], wb[RS[fu].cp]);
		add_writer(RS[fu].ret, val, fu, RS[fu].vir);
		//printf("add writer over\n");
	} else {
		//TODO+: 检查是否预测正确，若预测正确则将虚拟空间commit并将所有虚拟保留站实体化
		//		若检测不正确，则放弃所有虚拟保留站和虚拟空间
		if (predict_flag) {
			if (RS[fu].cp == branch) {
				//printf("get result on cp=%d with %d\n", RS[fu].cp, val);
				ptot ++;
				if (val == predict[branch]) {		//预测成功
					acc++;
					memcpy(reg, vreg, sizeof(reg));
					memcpy(rf, vrf, sizeof(rf));
					//实体化所有虚拟执行指令
					for(int i = 0; i < ADD_S+MULT_S+LOAD_S; i++) 
						if(RS[i].busy > 0 && RS[i].vir == true)
							RS[i].vir = false;
				} else {							//预测失败，到目标地址
					if (val == 1) cp_next = branch+RS[fu].ret; else cp_next = branch + 1;
					predict[branch] = val;			//修改预测值
					//清除所有虚拟执行指令
					for(int i = 0; i < ADD_S+MULT_S+LOAD_S; i++) 
						if(RS[i].busy > 0 && RS[i].vir == true) {
							RS[i].busy = 0;
							RS[i].vir = false;
							que[Instr2Op(RS[i].op)]--;
						}
				}
				//清除虚拟空间，理论上可以不清理
				memset(vreg, 0, sizeof(vreg));
				memset(vrf, -1, sizeof(vrf));

				vir = false;
				block = false;
			} else {
				printf("error occur when cp=%d is in calced but virtual at cp=%d\n", RS[fu].cp, branch);
			}
		} else {
			if (val == 1) cp_next = cp+RS[fu].ret; else cp_next = cp+1;
			block = false;
		}
		
	}
	if (RS[fu].busy) {
		RS[fu].busy = 0;
		que[Instr2Op(RS[fu].op)]--;
	}

	//printf("Excute finish\n");

}

void Tomasulo::add_writer(int _reg, int val, int fu, bool _vir) {
	WB[WB_top].reg = _reg;
	WB[WB_top].fu = fu;
	WB[WB_top].val = val;
	WB[WB_top].vir = _vir;
	WB_top++;
}

void Tomasulo::clean_writer() {
	//TODO+: 需要区分当前更新是虚拟更新还是现实更新，若为虚拟更新只更新虚拟空间，现实更新都要更新
	//

//	printf("cleaning +\n");
	for (int k = 0; k < WB_top; k++) {
		WriteEvent x = WB[k];
		if (x.vir == false) {			//如果x为真实数据广播
	//		printf("%d %d\n", x.fu, rf[x.reg]);
			if (x.fu == rf[x.reg]) {			
				reg[x.reg] = x.val;
				rf[x.reg] = -1;
			}
		}
		if (vir == true) {				//虚拟空间不论真实广播和虚拟广播都要接受并修改
			if (x.fu == vrf[x.reg]) {			
				vreg[x.reg] = x.val;
				vrf[x.reg] = -1;
			}
		}
		for (int i = 0; i < ADD_S+MULT_S+LOAD_S; i++) {
			if (RS[i].q1 == x.fu) RS[i].q1 = -1, RS[i].v1 = x.val;
			if (RS[i].q2 == x.fu) RS[i].q2 = -1, RS[i].v2 = x.val;
		}
	}
	WB_top = 0;
//	printf("cleaning -\n");
}

void Tomasulo::writer_preclean(int _reg, int _ind) {
	//TODO+: 需要区分当前更新是虚拟更新还是现实更新，若为虚拟更新只更新虚拟空间，现实更新都要更新
	for (int k = 0; k < WB_top; k++) {
		WriteEvent x = WB[k];
//		printf("%d %d\n", x.fu, rf[x.reg]);
		if (x.reg == _reg && x.fu == _ind) {
			if (x.vir == false) {			//如果x为真实数据广播
		//		printf("%d %d\n", x.fu, rf[x.reg]);
				if (x.fu == rf[x.reg]) {			
					reg[x.reg] = x.val;
					rf[x.reg] = -1;
				}
			}
			if (vir == true) {				//虚拟空间不论真实广播和虚拟广播都要接受并修改
				if (x.fu == vrf[x.reg]) {			
					vreg[x.reg] = x.val;
					vrf[x.reg] = -1;
				}
			}
			for (int i = 0; i < ADD_S+MULT_S+LOAD_S; i++) {
				if (RS[i].q1 == x.fu) RS[i].q1 = -1, RS[i].v1 = x.val;
				if (RS[i].q2 == x.fu) RS[i].q2 = -1, RS[i].v2 = x.val;
			}
			WB[k].reg = -1, WB[k].fu = -2;
		}
	}

}
void Tomasulo::show_regs(int lim) {
	printf("clock = %3d cp = %3d\n  ", clock, cp); 
	for(int i = 0; i < lim; i++) printf("R%d(%d): %d;   ", i, rf[i], reg[i]);
	printf("\n  ");
	if (vir) {
		for(int i = 0; i < lim; i++) printf("r%d(%d): %d;   ", i, vrf[i], vreg[i]);
		printf("\n");
	}
}

void Tomasulo::print(int flag) {		//0 normal 1 short 2 long
	if (flag == -1) return;
	if (flag == 1) {	//Brief
		printf("CLOCK = %d\tCP = %d\tUSE = %d\tQUE = %d\tBLOCK = %d\tVIR = %d\n", clock, cp, use[0]+use[1]+use[2], que[0]+que[1]+que[2], block, vir);
		return;
	}
	printf("=================================\nCLock = %4d\t\t CP = %4d(%4d)\nBlock = %d\t Vir = %d\n", clock, cp, cp_next, block, vir);
	printf("ID\tBUSY\tCP\top\tq1\tv1\tq2\tv2\tLEFT\tVIR\n");
	for (int i = 0; i < ADD_S+MULT_S+LOAD_S; i++)
		if (RS[i].busy != 0) 
			printf("%d\t%d\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\n", i, RS[i].busy, RS[i].cp, name[RS[i].op], RS[i].q1, RS[i].v1, RS[i].q2, RS[i].v2, (RS[i].tar != -1)? f[RS[i].tar].left: -1, RS[i].vir); 
	printf("Write back:\n");
	for (int i = 0; i < WB_top; i++)
		printf("(R%d(%d) = %d)\n", WB[i].reg, WB[i].fu, WB[i].val);
	if (flag == 2) {
		printf("Register:\n");
		for (int i = 0; i < 32; i++) {
			printf("R%2d:(%d)%10d\t", i, rf[i], reg[i]);
			if (i % 8 == 7) printf("\n");
		}
		printf("use = [%d %d %d]\n", use[0], use[1], use[2]);
		printf("que = [%d %d %d]\n", que[0], que[1], que[2]);
		printf("pendings:\n");
		for (int t = 0; t < 3; t++) {
			for (int i = pending[t].l; i != pending[t].r; i=(i+1)%pending[t].n) printf("%d\t", pending[t].Q[i]);
			printf("\n");
		}
	}
	printf("-------------------------\n");

}

// queue 维护
void queue::add(int x, station * cp) {
	Q[r] = x;
	C[r] = cp;
	r = (r+1) % n;
	//printf("after adding: (%d %d) ", l, r);
	//for (int i = 0; i < n; i++) printf("%d ", Q[i]); printf("\n");
}
int queue::pop() {
	int ret = l;
	for (int i = l; i != r; i = (i+1)%n) 
		if (C[i]->clock < C[ret]->clock || (C[i]->clock == C[ret]->clock && C[i]->cp < C[ret]->cp)) ret = i;
	int ans = Q[ret];
	Q[ret] = Q[l]; C[ret] = C[l]; l = (l+1) % n;
	return ans;
}

int queue::size() {
	return (r-l+n) % n;
}