#ifndef __YM12_H__
#define __YM12_H__

struct ym12_op_t {
	unsigned char op_data[8];
	long op_padding[2];
};

struct ym12_ch_t {
	struct ym12_op_t op[4];
	unsigned char algo;
	unsigned char fback;
	unsigned char padding[14];
};

struct ym12_t {
	struct ym12_ch_t ym;
	unsigned char name[16];
	unsigned char dumper[16];
	unsigned char game[16];
}KM2612;


/*
// operators data
for (op=0; op<4; op++) {
	KM2612.ym.op[op].op_data[0] = (UCHAR) (YM2612.REG[part][0x30 + chan + 4*op])&0xFF;
	KM2612.ym.op[op].op_data[1] = (UCHAR) (YM2612.REG[part][0x40 + chan + 4*op])&0xFF;
	KM2612.ym.op[op].op_data[2] = (UCHAR) (YM2612.REG[part][0x50 + chan + 4*op])&0xFF;
	KM2612.ym.op[op].op_data[3] = (UCHAR) (YM2612.REG[part][0x60 + chan + 4*op])&0xFF;
	KM2612.ym.op[op].op_data[4] = (UCHAR) (YM2612.REG[part][0x70 + chan + 4*op])&0xFF;
	KM2612.ym.op[op].op_data[5] = (UCHAR) (YM2612.REG[part][0x80 + chan + 4*op])&0xFF;
	KM2612.ym.op[op].op_data[6] = (UCHAR) (YM2612.REG[part][0x90 + chan + 4*op])&0xFF;
}

// channel data
KM2612.ym.algo = (UCHAR) (YM2612.REG[part][0xB0 + chan])&0x07;
KM2612.ym.fback = (UCHAR) (YM2612.REG[part][0xB0 + chan])&0x38;
KM2612.ym.fback >>=3;
*/

#endif