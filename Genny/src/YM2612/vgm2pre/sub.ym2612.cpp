#include "sub.ym2612.hpp"

#ifndef _YM2612_GYB_
#define _YM2612_GYB_
#endif

void SubYM2612::init() {
	unsigned i;
	for (i = 0; i<6; ++i) {
		memset(&insts[i], 0x00, sizeof(ins_t));
		memset(&instsPrev[i], 0xff, sizeof(ins_t));
	}
}
void SubYM2612::add(ins_t *ins, int fid) {
	ins_t i;
	memcpy(&i, ins, sizeof(ins_t));
	i.file_id = fid;
	insList.append(i);
}

int SubYM2612::find(ins_t *ins) {
	int i;
	if (!ins->ch.op[0].ar&&!ins->ch.op[1].ar&&!ins->ch.op[2].ar&&!ins->ch.op[3].ar) return -1;
	if (ins->ch.op[0].tl>0x7e&&ins->ch.op[1].tl>0x7e&&ins->ch.op[2].tl>0x7e&&ins->ch.op[3].tl>0x7e) return -1;
	for (i = 0; i<insList.size(); ++i) {
		if (!memcmp(ins, &insList[i], sizeof(ins_t))) return i;
	}
	return -1;
}

int SubYM2612::slot(ins_t *ins) {
	const int slotMap[8] = { 0x08,0x08,0x08,0x08,0x0c,0x0e,0x0e,0x0f };
	return slotMap[ins->ch.algo&7];
}

bool SubYM2612::compare(ins_t *ins1, ins_t *ins2, bool use_ext_cmp=false) {
	ym2612_op_t *op1, *op2;
	int i, sl;
	if (ins1->ch.algo!=ins2->ch.algo) return false;
	if (ins1->ch.fback!=ins2->ch.fback) return false;
	if (use_ext_cmp) {
		if (ins1->ch.pms!=ins2->ch.pms) return false;
		if (ins1->ch.ams!=ins2->ch.ams) return false;
	}
	sl = slot(ins1);
	if (!(sl&1)) if(ins1->ch.op[0].tl!=ins2->ch.op[0].tl) return false;
	if (!(sl&2)) if(ins1->ch.op[1].tl!=ins2->ch.op[1].tl) return false;
	if (!(sl&4)) if(ins1->ch.op[2].tl!=ins2->ch.op[2].tl) return false;
	if (!(sl&8)) if(ins1->ch.op[3].tl!=ins2->ch.op[3].tl) return false;
	for (i = 0; i<4; ++i) {
		op1 = &ins1->ch.op[i];
		op2 = &ins2->ch.op[i];
		if (op1->ar!=op2->ar) return false;
		if (op1->dr!=op2->dr) return false;
		if (op1->sr!=op2->sr) return false;
		if (op1->rr!=op2->rr) return false;
		if (op1->sl!=op2->sl) return false;
		if (op1->mul!=op2->mul) return false;
		if (op1->dt!=op2->dt) return false;
		if (op1->ssg!=op2->ssg) return false;
		if (op1->rs!=op2->rs) return false;
		if (use_ext_cmp&&op1->am!=op2->am) return false;
	}
	return true;
}

void SubYM2612::write(int bank, int reg, int val, int f) {
	ym2612_op_t *op;
	int dtTable[8]={0,1,2,3,0,-1,-2,-3};
	int ch;

	if (reg==0xb3) return;	// INVALID

	if (!bank) {
		switch (reg) {
			case 0x2b://DAC on/off
				dacOn = (val&0x80)?true:false;
				return;
			case 0x28://key on/off
				ch = val&7;
				if (ch>3) --ch;
				if (ch==5&&dacOn) return;
				if (val&0xf0) {
					if (memcmp(&instsPrev[ch],&insts[ch],sizeof(ins_t))) {
						if (find(&insts[ch])<0) add(&insts[ch],f);
					}
					memcpy(&instsPrev[ch],&insts[ch],sizeof(ins_t));
				}
				return;
		}
	}

	/**** early exits { ****/
	if (reg<0x30) return;
	if ((reg&3)==3) return;
	/**** } ****/

	ch = (reg&3)+bank*3;
	op = &insts[ch].ch.op[(reg>>2)&3];
	switch (reg&0xf0) {
		case 0x20: break;	// BANK 0 registers ignored
		case 0x30:	//DT1,MUL
			op->dt = (val>>4)&7;
			op->detune = dtTable[op->dt];
			op->mul = val&0x0f;
			break;
		case 0x40:	//TL
			op->tl = val&0x7f;
			break;
		case 0x50:	//RS,AR
			op->rs = (val>>6)&3;
			op->ar = val&0x1f;
			break;
		case 0x60:	//AM,D1R
			op->am = (val>>7)&1;
			op->dr = val&0x1f;
			break;
		case 0x70:	//D2R
			op->sr = val&0x1f;
			break;
		case 0x80:	//D1L,RR
			op->sl = val>>4;
			op->rr = val&0x0f;
			break;
		case 0x90:	//SSG-EG
			op->ssg = val&0x0f;
			break;
		case 0xb0:
			if (reg<0xb3) {//FB,ALGO
				insts[ch].ch.algo = val&7;
				insts[ch].ch.fback = (val>>3)&7;
			}
			else if (reg==0xb3) break;	// UNUSED
			else if (reg<0xb7) {//PAN,PMS,AMS
				insts[ch].ch.pms = val&7;
				insts[ch].ch.ams = (val>>4)&3;
			}
			break;
		case 0xa0: break;	// YM2612 set freq
		default: break;
	}
}

unsigned SubYM2612::group(bool use_ext_cmp=false) {
	unsigned u = 0;
	int j, id, i, z, v, mv, sl;
	z = insList.size();
	// initialize inst list IDs
	i = z; while (--i>=0) insList[i].id = -1;
	// search for same insts w/different volume
	for (i = 0; i<z; ++i) {
		if (insList[i].id<0) {
			insList[i].id = u++;
			for (j = 0; j<z; ++j) {
				if (i==j) continue;
				if (compare(&insList[i],&insList[j],use_ext_cmp)) insList[j].id = insList[i].id;
			}
		}
	}
	// search for loudest of same insts
	for (j = 0; j<u; ++j) {
		//_log({"Grouping #",j,"..."});
		id = -1;
		mv = 0;
		for (i = 0; i<z; ++i) {
			if (insList[i].id==j) {
				sl = slot(&insList[i]);
				v = 0;
				if (sl&1) v += (127-insList[i].ch.op[0].tl);
				if (sl&2) v += (127-insList[i].ch.op[1].tl);
				if (sl&4) v += (127-insList[i].ch.op[2].tl);
				if (sl&8) v += (127-insList[i].ch.op[3].tl);
				if (v>mv) {id = i; mv = v;}
			}
		}
		if (id>=0) {
			//_log({"found ",id,"\n"});
			for (i = 0; i<z; ++i) {
				if (insList[i].id==j) {
					if (i!=id) insList[i].id = -1;
				}
			}
		}
		else _log({"No master presets found for #",j," :(\n"});
	}
	return u;
}

void SubYM2612::process(const lstring &fl, const V2PConfiguration &cfg) {
	string fn_edit;
	unsigned j,i;
	unsigned z = insList.size();
	unsigned used = group(cfg.use_ext_cmp_ym2612);
	_log({"OK: ",used," OPN instrument(s) were found\n"});
	bank.version = 2;
	bank.melody.insts.reset();
	bank.drums.insts.reset();
	bool canDumpOne = false;
	for (unsigned cd=0; cd<N::OPN::Types::GYB; ++cd) {
		if (!cfg.disabledOPN[cd]&&cfg.dumpOPN[cd]) {canDumpOne = true; break;}
	}
	for (j = 0; j<used; ++j) {
		//_log({"Looking for ins ",j,"...\n"});
		for (i = 0; i<z; ++i) {
			//_log({"ins ",i," = ",insList[i].id,"\n"});
			if (insList[i].id==j) {
				if (insList[i].file_id>fl.size()) {
					_log({"File ID ",insList[i].file_id," is out of bounds (",fl.size(),"), skipping...\n"});
					continue;
				}
				fn_edit = {basename(fl[insList[i].file_id]),"_",j};
#ifdef _YM2612_GYB_
				if (!addToBank(insList[i],notdir(fn_edit),false,0)) {
					_log({"ERR: Can't add '",fn_edit,"' to OPN bank\n"});
				}
				else _log({"OK: Added '",fn_edit,"' to OPN bank\n"});
#endif
				//_log({"Saving ",notdir(fn_edit),"..."});
				if (canDumpOne) {
					if (!save(insList[i], fn_edit, cfg)) {
						_log({"ERR: Can't save file '",fn_edit,"'\n"});
						break;
					}
				}
				//else _log({"done!\n"});
			}
		}
	}
#ifdef _YM2612_GYB_
	fn_edit = {"bank-opn-",(unsigned)time(NULL)};
	if (!cfg.disabledOPN[N::OPN::Types::GYB]&&cfg.dumpOPN[N::OPN::Types::GYB]&&bank.melody.insts.size()>0) {
		if (!saveGYB(fn_edit)) {
			_log({"ERR: Can't save OPN Bank file '",fn_edit,"' of size ",bank.melody.insts.size(),"\n"});
		}
	}
#endif
}

bool SubYM2612::save(const ins_t &ins, const string &fn, const V2PConfiguration &cfg) {
	unsigned ret = 0;
	if (cfg.dumpOPN[N::OPN::Types::VGI]) {++ret; if (!saveVGI(ins, fn)) --ret;}
	if (cfg.dumpOPN[N::OPN::Types::TFI]) {++ret; if (!saveTFI(ins, fn)) --ret;}
	if (cfg.dumpOPN[N::OPN::Types::TYI]) {++ret; if (!saveTYI(ins, fn)) --ret;}
	if (cfg.dumpOPN[N::OPN::Types::EIF]) {++ret; if (!saveEIF(ins, fn)) --ret;}
	if (cfg.dumpOPN[N::OPN::Types::DMP]) {++ret; if (!saveDMP(ins, fn)) --ret;}
	if (cfg.dumpOPN[N::OPN::Types::Y12]) {++ret; if (!saveY12(ins, fn)) --ret;}
	//if (cfg.dumpOPN[N::OPN::Types::GYB]) {++ret; if (!saveGYB(fn)) --ret;}
	return ret>0;
}

bool SubYM2612::saveVGI(const ins_t &ins, const string &fn) {
	uint8_t dataVGI[43] = {0};
	unsigned aa = 0, pp = 0;
	dataVGI[pp++] = ins.ch.algo;
	dataVGI[pp++] = ins.ch.fback;
	dataVGI[pp++] = ins.ch.pms|(ins.ch.ams<<4);
    for (aa = 0; aa<4; ++aa) {
		dataVGI[pp++] = ins.ch.op[aa].mul;
		dataVGI[pp++] = ins.ch.op[aa].detune+3;
		dataVGI[pp++] = ins.ch.op[aa].tl;
		dataVGI[pp++] = ins.ch.op[aa].rs;
		dataVGI[pp++] = ins.ch.op[aa].ar;
		dataVGI[pp++] = ins.ch.op[aa].dr|(ins.ch.op[aa].am<<7);
		dataVGI[pp++] = ins.ch.op[aa].sr;
		dataVGI[pp++] = ins.ch.op[aa].rr;
		dataVGI[pp++] = ins.ch.op[aa].sl;
		dataVGI[pp++] = ins.ch.op[aa].ssg;
	}
	return file::write({fn,".vgi"}, dataVGI, sizeof(dataVGI));
}

bool SubYM2612::saveTFI(const ins_t &ins, const string &fn) {
	uint8_t dataTFI[42] = {0};
	unsigned aa = 0, pp = 0;
	dataTFI[pp++] = ins.ch.algo;
	dataTFI[pp++] = ins.ch.fback;
    for (aa = 0; aa<4; ++aa) {
		dataTFI[pp++] = ins.ch.op[aa].mul;
		dataTFI[pp++] = ins.ch.op[aa].detune+3;
		dataTFI[pp++] = ins.ch.op[aa].tl;
		dataTFI[pp++] = ins.ch.op[aa].rs;
		dataTFI[pp++] = ins.ch.op[aa].ar;
		dataTFI[pp++] = ins.ch.op[aa].dr;
		dataTFI[pp++] = ins.ch.op[aa].sr;
		dataTFI[pp++] = ins.ch.op[aa].rr;
		dataTFI[pp++] = ins.ch.op[aa].sl;
		dataTFI[pp++] = ins.ch.op[aa].ssg;
	}
	return file::write({fn,".tfi"}, dataTFI, sizeof(dataTFI));
}

bool SubYM2612::saveTYI(const ins_t &ins, const string &fn) {
	//_log({" ",fn,".tyi"});
	uint8_t dataTYI[32] = {0};
	unsigned aa = 0, pp = 0;
	/*for (aa = 0; aa<4; ++aa) {dataTYI[pp++] = ins.ch.op[aa].mul|(ins.ch.op[aa].dt<<4);}
	for (aa = 0; aa<4; ++aa) {dataTYI[pp++] = ins.ch.op[aa].tl;}
	for (aa = 0; aa<4; ++aa) {dataTYI[pp++] = ins.ch.op[aa].ar|(ins.ch.op[aa].rs<<6);}
	for (aa = 0; aa<4; ++aa) {dataTYI[pp++] = ins.ch.op[aa].dr|(ins.ch.op[aa].am<<7);}
	for (aa = 0; aa<4; ++aa) {dataTYI[pp++] = ins.ch.op[aa].sr;}
	for (aa = 0; aa<4; ++aa) {dataTYI[pp++] = ins.ch.op[aa].rr|(ins.ch.op[aa].sl<<4);}
	for (aa = 0; aa<4; ++aa) {dataTYI[pp++] = ins.ch.op[aa].ssg;}*/
//
	for (aa = 0; aa<4; ++aa) {
		dataTYI[aa] = ins.ch.op[aa].mul|(ins.ch.op[aa].dt<<4);
		dataTYI[aa+4] = ins.ch.op[aa].tl;
		dataTYI[aa+8] = ins.ch.op[aa].ar|(ins.ch.op[aa].rs<<6);
		dataTYI[aa+12] = ins.ch.op[aa].dr|(ins.ch.op[aa].am<<7);
		dataTYI[aa+16] = ins.ch.op[aa].sr;
		dataTYI[aa+20] = ins.ch.op[aa].rr|(ins.ch.op[aa].sl<<4);
		dataTYI[aa+24] = ins.ch.op[aa].ssg;
		//++pp;
	}
//
	dataTYI[28] = ins.ch.algo|(ins.ch.fback<<3);
	dataTYI[29] = ins.ch.pms|(ins.ch.ams<<4);
	dataTYI[30] = 'Y';
	dataTYI[31] = 'I';
	return file::write({fn,".tyi"}, dataTYI, sizeof(dataTYI));
}

bool SubYM2612::saveY12(const ins_t &ins, const string &fn) {
	//_log({" ",fn,".y12"});
	uint8_t dataY12[128] = {0};
	//return file::write({fn,".y12"}, dataY12, sizeof(dataY12));
	unsigned aa = 0, pp = 0;
	for (aa = 0; aa<4; ++aa) {
		dataY12[pp++] = ins.ch.op[aa].mul|(ins.ch.op[aa].dt<<4);
		dataY12[pp++] = ins.ch.op[aa].tl;
		dataY12[pp++] = ins.ch.op[aa].ar|(ins.ch.op[aa].rs<<6);
		dataY12[pp++] = ins.ch.op[aa].dr|(ins.ch.op[aa].am<<7);	// AM IS UNUSED
		dataY12[pp++] = ins.ch.op[aa].sr;
		dataY12[pp++] = ins.ch.op[aa].rr|(ins.ch.op[aa].sl<<4);
		dataY12[pp++] = ins.ch.op[aa].ssg;	// UNUSED
		// 9 UNUSED BYTES FOLLOW
		++pp;
		++pp;++pp;++pp;++pp;++pp;++pp;++pp;++pp;
	}
	dataY12[pp++] = ins.ch.algo;
	dataY12[pp++] = ins.ch.fback;
	// 14 UNUSED BYTES FOLLOW
	++pp;++pp;++pp;++pp;++pp;++pp;++pp;++pp; ++pp;++pp;++pp;++pp;++pp;++pp;
	// TODO: 16 BYTES FOR NAME
	++pp;++pp;++pp;++pp;++pp;++pp;++pp;++pp; ++pp;++pp;++pp;++pp;++pp;++pp;++pp;++pp;
	// TODO: 16 BYTES FOR DUMPER
	memcpy(&dataY12[pp],"VGM2Pre",7);
	// TODO 16 BYTES FOR GAME
	return file::write({fn,".y12"}, dataY12, sizeof(dataY12));
}

bool SubYM2612::saveEIF(const ins_t &ins, const string &fn) {
	//_log({" ",fn,".eif"});
	uint8_t dataEIF[29] = {0};
	unsigned aa = 0, pp = 0;
	dataEIF[pp++] = ins.ch.algo|(ins.ch.fback<<3);
	for (aa = 0; aa<4; ++aa) {dataEIF[pp++] = ins.ch.op[aa].mul|(ins.ch.op[aa].dt<<4);}
	for (aa = 0; aa<4; ++aa) {dataEIF[pp++] = ins.ch.op[aa].tl;}
	for (aa = 0; aa<4; ++aa) {dataEIF[pp++] = ins.ch.op[aa].ar|(ins.ch.op[aa].rs<<6);}
	for (aa = 0; aa<4; ++aa) {dataEIF[pp++] = ins.ch.op[aa].dr|(ins.ch.op[aa].am<<7);}
	for (aa = 0; aa<4; ++aa) {dataEIF[pp++] = ins.ch.op[aa].sr;}
	for (aa = 0; aa<4; ++aa) {dataEIF[pp++] = ins.ch.op[aa].rr|(ins.ch.op[aa].sl<<4);}
	for (aa = 0; aa<4; ++aa) {dataEIF[pp++] = ins.ch.op[aa].ssg;}
	return file::write({fn,".eif"}, dataEIF, sizeof(dataEIF));
	//return false;
}
bool SubYM2612::saveDMP(const ins_t &ins, const string &fn) {
	//_log({" ",fn,".dmp"});
	unsigned aa, pp = 0;
	uint8_t dv = 7, ds = dv==7?2+4+1+4*11:2+8+4*19;
	uint8_t dataDMP[ds];
	if (dv==7) {	// DMP v7
		dataDMP[pp++] = 7;	// older DefleMask instrument
		dataDMP[pp++] = 1;	// type = FM
		dataDMP[pp++] = 1;	// 0 = 2-op, 1 = 4-op
		dataDMP[pp++] = ins.ch.pms;
		dataDMP[pp++] = ins.ch.fback;
		dataDMP[pp++] = ins.ch.algo;
		dataDMP[pp++] = ins.ch.ams;
		for (aa = 0; aa<4; ++aa) {
			dataDMP[pp++] = ins.ch.op[aa].mul;
			dataDMP[pp++] = ins.ch.op[aa].tl;
			dataDMP[pp++] = ins.ch.op[aa].ar;
			dataDMP[pp++] = ins.ch.op[aa].dr;
			dataDMP[pp++] = ins.ch.op[aa].sl;
			dataDMP[pp++] = ins.ch.op[aa].rr;
			dataDMP[pp++] = ins.ch.op[aa].am;
			// YM2612-specific
			dataDMP[pp++] = ins.ch.op[aa].rs;
			dataDMP[pp++] = ins.ch.op[aa].detune+3;
			dataDMP[pp++] = ins.ch.op[aa].sr;
			dataDMP[pp++] = ins.ch.op[aa].ssg;
		}
	}
	else {	// DMP v16
		dataDMP[pp++] = 16;	// DefleMask 9
		dataDMP[pp++] = 1;	// type = FM
		dataDMP[pp++] = ins.ch.algo; dataDMP[pp++] = 0;
		dataDMP[pp++] = ins.ch.fback; dataDMP[pp++] = 0;
		dataDMP[pp++] = ins.ch.pms; dataDMP[pp++] = 0;
		dataDMP[pp++] = 1; dataDMP[pp++] = ins.ch.ams;
		for (aa = 0; aa<4; ++aa) {
			dataDMP[pp++] = ins.ch.op[aa].am;
			dataDMP[pp++] = ins.ch.op[aa].ar;
			dataDMP[pp++] = 0;
			dataDMP[pp++] = ins.ch.op[aa].dr;
			dataDMP[pp++] = 0;
			dataDMP[pp++] = 0;
			dataDMP[pp++] = 0;
			dataDMP[pp++] = 0;
			dataDMP[pp++] = ins.ch.op[aa].mul;
			dataDMP[pp++] = ins.ch.op[aa].rr;
			dataDMP[pp++] = ins.ch.op[aa].sl;
			dataDMP[pp++] = 0;
			dataDMP[pp++] = ins.ch.op[aa].tl;
			dataDMP[pp++] = 0;
			dataDMP[pp++] = 0;
			dataDMP[pp++] = ins.ch.op[aa].rs;
			dataDMP[pp++] = ins.ch.op[aa].detune;
			dataDMP[pp++] = ins.ch.op[aa].sr;
			dataDMP[pp++] = ins.ch.op[aa].ssg;
		}
	}
	return file::write({fn,".dmp"}, dataDMP, sizeof(dataDMP));
	//return false;
}

#ifdef _YM2612_GYB_
bool SubYM2612::addToBank(const ins_t &ins, const string &fn, bool as_drum=false, uint8_t extra=0) {
	gyb_ins_t gins;
	gins.name = fn;
	unsigned aa = 0, pp = 0;
	uint8_t s;
	for (aa = 0; aa<4; ++aa) {gins.reg[pp++] = ins.ch.op[aa].mul|(ins.ch.op[aa].dt<<4);}
	for (aa = 0; aa<4; ++aa) {gins.reg[pp++] = ins.ch.op[aa].tl;}
	for (aa = 0; aa<4; ++aa) {gins.reg[pp++] = ins.ch.op[aa].ar|(ins.ch.op[aa].rs<<6);}
	for (aa = 0; aa<4; ++aa) {gins.reg[pp++] = ins.ch.op[aa].dr|(ins.ch.op[aa].am<<7);}
	for (aa = 0; aa<4; ++aa) {gins.reg[pp++] = ins.ch.op[aa].sr;}
	for (aa = 0; aa<4; ++aa) {gins.reg[pp++] = ins.ch.op[aa].rr|(ins.ch.op[aa].sl<<4);}
	for (aa = 0; aa<4; ++aa) {gins.reg[pp++] = ins.ch.op[aa].ssg;}
	gins.reg[pp++] = ins.ch.algo|(ins.ch.fback<<3);
	gins.reg[pp++] = ins.ch.pms|(ins.ch.ams<<4)|0xc0;
	gins.reg[pp++] = extra;	// as melody: transpose, loads as (x+128)%256; as drum: note map
	gins.reg[pp++] = 0;	// padding byte
	if (as_drum) {
		s = bank.drums.insts.size();
		if (s<256) {
			if (s<128) bank.drums.instMap[s] = s;
			bank.drums.insts.append(gins);
		}
		else return false;
	}
	else {
		s = bank.melody.insts.size();
		if (s<256) {
			if (s<128) bank.melody.instMap[s] = s;
			bank.melody.insts.append(gins);
		}
		else return false;
	}
	return true;
}
bool SubYM2612::saveGYB(const string &fn) {
	unsigned fs = 262;//3+2+2*0x80+1;	// Header + Ins Count + Mappings + Ins Data + Ins Names + CheckSum
	unsigned aa = 0, pp;
	unsigned len;
	unsigned vb = bank.version==1?0x1e:0x20;
	//for (aa = 0; aa<bank.melody.insts.size(); ++aa) {
	vector<string> tmpArr;
	for (auto &inst : bank.melody.insts) {
		//if (bank.version==1) fs += 0x1e;
		//else fs += 0x20;
		tmpArr.append(inst.name);
		len = inst.name.length(); if (len>255) len = 255;
		fs += vb+1+len;
		//fs += 1+bank.melody.insts[aa].name.length();
	}
	for (auto &inst : bank.drums.insts) {
		//if (bank.version==1) fs += 0x1e;
		//else fs += 0x20;
		tmpArr.append(inst.name);
		len = inst.name.length(); if (len>255) len = 255;
		fs += vb+1+len;
	}
	fs += 4;
	uint8_t dataGYB[fs];
	pp = 0;
	dataGYB[pp++] = 26; dataGYB[pp++] = 12;
	dataGYB[pp++] = bank.version;
	dataGYB[pp++] = bank.melody.insts.size();
	dataGYB[pp++] = bank.drums.insts.size();
	for (aa = 0; aa<128; ++aa) {
		dataGYB[pp++] = bank.melody.instMap[aa];
		dataGYB[pp++] = bank.drums.instMap[aa];
	}
	if (bank.version==2) {
		dataGYB[pp++] = bank.lfo;
	}
	//for (aa = 0; aa<bank.melody.insts.size(); ++aa) {
	for (auto &inst : bank.melody.insts) {
		if (bank.version==1) {
			memcpy(dataGYB+pp, inst.reg, 0x1d);
			dataGYB[pp+0x1d] = inst.reg[0x1e];
			pp += 0x1e;
		}
		else {
			memcpy(dataGYB+pp, inst.reg, 0x20);
			pp += 0x20;
		}
		//len = inst.name.length(); if (len>255) len = 255;
		//uint8_t tmp[len+1]; tmp[0] = len&0xff;
		//memcpy(tmp+1, (const char*)inst.name, len);
		// TODO: copy substr(inst[aa].name, 0, min(inst[aa].name.length(), 255)) into tempArr
		//tmpArr.append(inst.name);
	}
	//for (aa = 0; aa<bank.drums.insts.size(); ++aa) {
	for (auto &inst : bank.drums.insts) {
		if (bank.version==1) {
			memcpy(dataGYB+pp, inst.reg, 0x1d);
			dataGYB[pp+0x1d] = inst.reg[0x1e];
			pp += 0x1e;
		}
		else {
			memcpy(dataGYB+pp, inst.reg, 0x20);
			pp += 0x20;
		}
		//len = idrm.name.length(); if (len>255) len = 255;
		//uint8_t tmp[len+1]; tmp[0] = len&0xff;
		//memcpy(tmp+1, (const char*)idrm.name, len);
		// TODO: copy substr(inst[aa].name, 0, min(inst[aa].name.length(), 255)) into tempArr
		//tmpArr.append(idrm.name);
	}
	// TODO: append tempArr to dataGYB
	for (auto &t : tmpArr) {
		len = t.length(); if (len>255) len = 255;
		dataGYB[pp++] = len;
		memcpy(dataGYB+pp, t.data(), len);
		pp += len;
	}
	uint32_t chk = gyb_file_t::calc_checksum(pp, dataGYB);
	if (pp<fs) dataGYB[pp] = 0;
	//memcpy(dataGYB+pp, 0x0, 0x04);
	//memcpy(dataGYB+pp, chk, 0x04);
	return file::write({fn,".gyb"}, dataGYB, sizeof(dataGYB));
	//return false;
}
#endif
