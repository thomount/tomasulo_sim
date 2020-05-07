#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "sim.h"
using namespace std;

void test(Sim * sim, const char* inf, const char* outf) {
	FILE * fi = fopen(inf, "r");
	FILE * fo = fopen(outf, "w");
	sim->config(fi, fo);
	sim->init();
	sim->run();
	fclose(fi);
	fclose(fo);
}

int main(int argc, char ** argv) {
	//printf("%d\n", argc);
	//根据不同的argv解析不同的运行指令：base、extend、performance
	string Idir = "../input/", Odir = "../output/", Itail = ".nel", Otail = ".log";
	vector<string> base = {string("0.basic"), string("1.basic"), string("2.basic"), string("3.basic"), string("4.basic")};
	vector<string> ext = {string("Fact"), string("Fabo"), string("Example"), string("Gcd")};
	vector<string> perf = {string("Mul"), string("Big_test")};
	vector<string> use;
	use.clear();
	Sim * sim = new Sim();
	if (argc == 1 || string(argv[1]) == "-b") {		//base
		use = base;
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
	//printf("use size = %d\n", use.size());
	for (string i: use) {
		printf("%s test start\n", i.c_str());
		test(sim, (Idir+i+Itail).c_str(), (Odir+i+Otail).c_str());
		printf("%s test end\n", i.c_str());
	}
}
