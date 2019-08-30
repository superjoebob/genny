#ifndef __SUB_YM2151_H__
#define __SUB_YM2151_H__

#include <nall/nall.hpp>
using namespace nall;

#include "fmt/pre.ym2151.hpp"
#include "config.hpp"

struct SubYM2151 {
	struct ins_t {
		int id;
		int file_id;
		ym2151_ch_t ch;
	};
	ins_t insts[8];
	ins_t instsPrev[8];
	vector<ins_t> insList;
	//int insListCount;
	int insListSize;
	bool dacOn;
	void init();
	void add(ins_t *ins, int file_id);
	int find(ins_t *ins);
	int slot(ins_t *ins);
	bool compare(ins_t *ins1, ins_t *ins2, bool use_ext_cmp);
	void write(int reg, int val, int f);
	unsigned group(bool use_ext_cmp);
	void process(const lstring &fl, const V2PConfiguration &cfg);
	bool save(const ins_t &ins, const string &fn, const V2PConfiguration &cfg);
	string toString(const ins_t &ins, unsigned num, const string &fn);
	bool saveOPM(const ins_t &ins, const string &fn);
};

#endif