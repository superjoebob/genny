#ifndef __PRE_YM2151_H__
#define __PRE_YM2151_H__

struct ym2151_op_t {
	unsigned char dt;	// 3-bit
	//unsigned char detune;
	unsigned char dt2;	// 2-bit
	unsigned char mul;
	unsigned char tl;
	unsigned char ks;
	unsigned char ar;
	unsigned char am;
	unsigned char dr;
	unsigned char sr;
	unsigned char sl;
	unsigned char rr;
	unsigned char ssg;
};

struct ym2151_ch_t {
	struct ym2151_op_t op[4];
	unsigned char algo;
	unsigned char fback;
	unsigned char pms;
	unsigned char ams;
	unsigned char noise;
	unsigned char nfreq;
	unsigned char lfreq;
	unsigned char lwave;
	unsigned char pmd;
	unsigned char amd;
};


#endif