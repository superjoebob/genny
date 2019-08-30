#ifndef __SUB_YM2612_H__
#define __SUB_YM2612_H__

#include <nall/nall.hpp>
using namespace nall;

#include "fmt/pre.ym2612.hpp"
#include "config.hpp"

#ifndef _YM2612_GYB_
#define _YM2612_GYB_
#endif

struct SubYM2612 {
	struct ins_t {
		int id;
		int file_id;
		ym2612_ch_t ch;
	};
	ins_t insts[6];
	ins_t instsPrev[6];
	vector<ins_t> insList;
	gyb_file_t bank;	// FOR GYB
	//int insListCount;
	int insListSize;
	bool dacOn;
	void init();
	void add(ins_t *ins, int file_id);
	int find(ins_t *ins);
	int slot(ins_t *ins);
	bool compare(ins_t *ins1, ins_t *ins2, bool use_ext_cmp);
	void write(int bank, int reg, int val, int f);
	unsigned group(bool use_ext_cmp);
	void process(const lstring &fl, const V2PConfiguration &cfg);
	bool save(const ins_t &ins, const string &fn, const V2PConfiguration &cfg);
	bool saveVGI(const ins_t &ins, const string &fn);
	bool saveTFI(const ins_t &ins, const string &fn);
	bool saveTYI(const ins_t &ins, const string &fn);
	bool saveY12(const ins_t &ins, const string &fn);
	bool saveEIF(const ins_t &ins, const string &fn);
	bool saveDMP(const ins_t &ins, const string &fn);
#ifdef _YM2612_GYB_
	bool addToBank(const ins_t &ins, const string &fn, bool as_drum, uint8_t extra);
	bool saveGYB(const string &fn);
#endif
	private:
		void _log(const string &s) {print(s);}
};

#endif