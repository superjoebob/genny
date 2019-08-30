#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "common.hpp"

struct V2PConfiguration {
	bool headless;
	// YM2612
	bool dumpOPN[N::OPN::Types::Total] = {0};
	bool disabledOPN[N::OPN::Types::Total] = {0};
	/*bool dump_ym2612_vgi;
	bool dump_ym2612_tfi;
	bool dump_ym2612_tyi;
	bool dump_ym2612_y12;
	bool dump_ym2612_eif;
	bool dump_ym2612_dmp;
	bool dump_ym2612_gyb;*/
	// YM2151
	bool dumpOPM[N::OPM::Types::Total] = {0};
	bool disabledOPM[N::OPM::Types::Total] = {0};
	//bool dump_ym2151_opm;
	// USE EXTENDED COMPARISON
	bool use_ext_cmp_ym2612;
	bool use_ext_cmp_ym2151;
};

#endif