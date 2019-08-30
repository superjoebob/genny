#ifndef __PRE_YM2612_H__
#define __PRE_YM2612_H__

struct ym2612_op_t {
	unsigned char dt;
	unsigned char detune;
	unsigned char mul;
	unsigned char tl;
	unsigned char rs;
	unsigned char ar;
	unsigned char am;
	unsigned char dr;
	unsigned char sr;
	unsigned char sl;
	unsigned char rr;
	unsigned char ssg;
};

struct ym2612_ch_t {
	struct ym2612_op_t op[4];
	unsigned char algo;
	unsigned char fback;
	unsigned char pms;
	unsigned char ams;
};


#endif