#ifndef __OPM_H__
#define __OPM_H__

#include "pre.ym2151.hpp"

/*******
//MiOPMdrv sound bank Paramer Ver2002.04.22
//LFO: LFRQ AMD PMD WF NFRQ
//@:[Num] [Name]
//CH: PAN	FL CON AMS PMS SLOT NE
//[OPname]: AR D1R D2R	RR D1L	TL	KS MUL DT1 DT2 AMS-EN

*******/

struct opm_t {
	string name;
	ym2151_ch_t ch;
};

#endif