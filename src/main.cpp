#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <ctime>
#include "sim.h"
using namespace std;

void test(Sim * sim, const char* inf, const char* outf) {
	FILE * fi = fopen(inf, "r");
	FILE * fo = fopen(outf, "w");
	//fprintf(0, "test\n");
	//printf("%d %d\n", fi, fo);
	//printf("%s %s\n", inf, outf);
	//printf("");
	sim->config(fi, fo);
	sim->init();
	sim->run();
	//printf("run over");
	fclose(fi);
	fclose(fo);
}

int main(int argc, char ** argv) {
	//printf("%d\n", argc);
	//根据不同的argv解析不同的运行指令：base、extend、performance
	string Idir = "../input/", Odir = "../log/", Itail = ".nel", Otail = ".log";
	vector<string> base = {string("0.basic"), string("1.basic"), string("2.basic"), string("3.basic"), string("4.basic")};
	vector<string> ext = {string("Fact"), string("Fabo"), string("Example"), string("Gcd")};
	vector<string> perf = {string("Mul"), string("Big_test")};
	vector<string> example = {string("mytest_1"), string("mytest_2")};
	vector<string> use;
	use.clear();
	Sim * sim = 0;

	string method = "";
	if (argc == 1 || string(argv[1]) == "-b") {		//base
		use = base;
	}
	sim = new Tomasulo(false);
	if (argc > 2 && string(argv[2]) == "-r") {		//base + ext
		if (sim) delete sim;
		sim = new Sim();
		method = "_r";
		//use.insert(use.end(), ext.begin(), ext.end());
	}
	if (argc > 2 && string(argv[2]) == "-p") {		//perform
		if (sim) delete sim;
		sim = new Tomasulo(true);
		method = "_p";
	}
	
	if (argc > 1 && string(argv[1]) == "-e") {		//base + ext
		use = base;
		use.insert(use.end(), ext.begin(), ext.end());
	}
	if (argc > 1 && string(argv[1]) == "-p") {		//perform
		use = perf;
	}
	if (argc > 1 && string(argv[1]) == "-a") {		//all
		use = base;
		use.insert(use.end(), ext.begin(), ext.end());
		use.insert(use.end(), perf.begin(), perf.end());

	}
	if (argc > 1 && string(argv[1]) == "-t") {		//perform
		use = example;
	}
	if (argc > 1 && string(argv[1]) == "-test") {
		Odir += "2017011315_";
		use = example;
		sim = new Tomasulo(true);
		use = base;
		use.insert(use.end(), ext.begin(), ext.end());
		use.insert(use.end(), perf.begin(), perf.end());
		use.insert(use.end(), example.begin(), example.end());
		method = "";
		sim->printConfig = 1;
	}
	if (argc > 1 && string(argv[1]) == "-detail") {
		Odir += "2017011315_";
		use = example;
		sim = new Tomasulo(true);
		use = base;
		use.insert(use.end(), ext.begin(), ext.end());
		use.insert(use.end(), perf.begin(), perf.end());
		use.insert(use.end(), example.begin(), example.end());
		method = "";
		sim->printConfig = 2;
	}
	printf("test set size = %d\n", use.size());
	for (string i: use) {
		printf("%s test testing...", i.c_str());
		int st = clock();
		test(sim, (Idir+i+Itail).c_str(), (Odir+i+method+Otail).c_str());
		printf("\t using time = %.3lfms\n", 1000.0*(clock()-st)/CLOCKS_PER_SEC);
	}
}
