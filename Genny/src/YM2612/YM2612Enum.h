#pragma once
#include <map>
enum YM2612Clock
{
	YM2612_NTSC = 7670453,
	YM2612_PAL = 7600489,
	YM2612_GENNY = 8000000
};
	  



//Don't mess with this unless you know what you're doing,
//these correspond to parameters on the actual YM2612 hardware
//and *probably* arent going to change. 
enum YM2612Param
{
	YM_DT = 0,
	YM_MUL = 1,
	YM_TL = 2,
	YM_DRUMTL = 3,
	YM_KS = 4,
	YM_AR = 5,
	YM_DR = 6,
	YM_SR = 7,
	YM_AM = 8,
	YM_SL = 9,
	YM_RR = 10,
	YM_F1 = 11,
	YM_F2 = 12,
	YM_SSG = 13,
	YM_AMS = 14,
	YM_FMS = 15,
	YM_L_EN = 16,
	YM_R_EN = 17,
	YM_FRQ = 18,
	YM_FB = 19,

	SN_SR = 20,
	SN_PERIODIC = 21,
	SN_DT = 22,

	YM_ALG = 23,
	YM_LFO_EN = 24,
	YM_LFO = 25, 
	YM_SPECIAL = 26,
	YM_FNUM1 = 27,
	YM_FNUM2 = 28,
	YM_NOTEON = 29,
	YM_DACEN = 30,
	YM_DAC = 31,
	YM_NONE
}; 


enum YM2612REG
{
	YMR_NOTEON = 0x28,

	YMR_DT1_MUL_OP1 = 0x30,
	YMR_DT1_MUL_OP2 = 0x34,
	YMR_DT1_MUL_OP3 = 0x38,
	YMR_DT1_MUL_OP4 = 0x3C,

	YMR_TL_OP1 = 0x40,
	YMR_TL_OP2 = 0x44,
	YMR_TL_OP3 = 0x48,
	YMR_TL_OP4 = 0x4C,

	YMR_RS_AR_OP1 = 0x50,
	YMR_RS_AR_OP2 = 0x54,
	YMR_RS_AR_OP3 = 0x58,
	YMR_RS_AR_OP4 = 0x5C,

	YMR_AM_D1R_OP1 = 0x60,
	YMR_AM_D1R_OP2 = 0x64,
	YMR_AM_D1R_OP3 = 0x68,
	YMR_AM_D1R_OP4 = 0x6C,

	YMR_D2R_OP1 = 0x70,
	YMR_D2R_OP2 = 0x74,
	YMR_D2R_OP3 = 0x78,
	YMR_D2R_OP4 = 0x7C,

	YMR_D1L_RR_OP1 = 0x80,
	YMR_D1L_RR_OP2 = 0x84,
	YMR_D1L_RR_OP3 = 0x88,
	YMR_D1L_RR_OP4 = 0x8C,

	YMR_SSG_OP1 = 0x90,
	YMR_SSG_OP2 = 0x94,
	YMR_SSG_OP3 = 0x98,
	YMR_SSG_OP4 = 0x9C,

	YMR_FB_ALG = 0xB0,
	YMR_AMS_FMS = 0xB4,
};

static unsigned char YM2612Param_getRange(YM2612Param param)
{
	switch(param)
	{
	case YM_DT:			return 7;
	case YM_MUL:		return 15;
	case YM_TL:			return 127;
	case YM_DRUMTL:		return 127;
	case YM_KS:			return 3;
	case YM_AR:			return 31;
	case YM_DR:			return 31;
	case YM_SR:			return 31;
	case YM_AM:			return 1;
	case YM_SL:			return 15;
	case YM_RR:			return 15;
	case YM_F1:			return 254;
	case YM_F2:			return 254;
	case YM_SSG:		return 8;
	case YM_FB:			return 7;
	case YM_L_EN:       return 1;
	case YM_R_EN:       return 1;
	case YM_AMS:		return 3;
	case YM_FMS:		return 7;
	case YM_FRQ:		return 48;
	case SN_SR:			return 3;
	case SN_PERIODIC:	return 1;
	case SN_DT:			return 100;
	case YM_ALG:		return 7;
	case YM_LFO:		return 7;
	case YM_LFO_EN:		return 1;
	case YM_SPECIAL:	return 1;
	}
	return 0;
}

static std::string YM2612Param_getName(YM2612Param param)
{
	switch(param)
	{
	case YM_DT:			return "Detune";
	case YM_MUL:		return "Freq Mul";
	case YM_TL:			return "Level";
	case YM_DRUMTL:		return "Drum Level";
	case YM_KS:			return "Key Scaling";
	case YM_AR:			return "Attack";
	case YM_DR:			return "Decay";
	case YM_SR:			return "Decay 2";
	case YM_AM:			return "LFO Enable";
	case YM_SL:			return "Sustain Level";
	case YM_RR:			return "Release";
	case YM_F1:			return "F1";
	case YM_F2:			return "F2";
	case YM_SSG:		return "SSG-EG";
	case YM_FB:			return "Feedback";
	case YM_L_EN:       return "L Enable";
	case YM_R_EN:       return "R Enable";
	case YM_AMS:		return "LFO AMS";
	case YM_FMS:		return "LFO FMS";
	case YM_FRQ:		return "Frequency";
	case SN_SR:			return "Shift Rate";
	case SN_PERIODIC:	return "Periodic Noise";
	case SN_DT:			return "Detune";
	case YM_ALG:		return "Algorithm";
	case YM_LFO:		return "LFO";
	case YM_LFO_EN:		return "Global LFO Enable";
	case YM_SPECIAL:	return "Special Mode";
	}
	return 0;
}

