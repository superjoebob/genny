#ifndef __TFI_H__
#define __TFI_H__

struct tfi_op_t {
	unsigned char multiple;//0..15
	unsigned char detune;//-3..0..3
	unsigned char totalLevel;//0..127
	unsigned char rateScale;//0..3
	unsigned char envAttack;//0..31
	unsigned char envDecay;//0..31
	unsigned char envSustain;//0..31
	unsigned char envRelease;//0..15
	unsigned char envRelLevel;//0..15
	unsigned char envType;//7..15
};

struct tfi_ch_t {
	unsigned char algo;//0..7
	unsigned char feedback;//0..7
	tfi_op_t op[4];
};

/*
tfi_ch_t insChn[6];
tfi_ch_t insChnPrev[6];
tfi_ch_t *insList;
*/

#endif