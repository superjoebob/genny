#include "sub.ym2151.hpp"

void SubYM2151::init() {
	unsigned i;
	for (i = 0; i<8; ++i) {
		memset(&insts[i], 0x00, sizeof(ins_t));
		memset(&instsPrev[i], 0xff, sizeof(ins_t));
	}
}
void SubYM2151::add(ins_t *ins, int fid) {
	ins_t i;
	memcpy(&i, ins, sizeof(ins_t));
	i.file_id = fid;
	insList.append(i);
}

int SubYM2151::find(ins_t *ins) {
	int i;
	if (!ins->ch.op[0].ar&&!ins->ch.op[1].ar&&!ins->ch.op[2].ar&&!ins->ch.op[3].ar) return -1;
	if (ins->ch.op[0].tl>0x7e&&ins->ch.op[1].tl>0x7e&&ins->ch.op[2].tl>0x7e&&ins->ch.op[3].tl>0x7e) return -1;
	for (i = 0; i<insList.size(); ++i) {
		if (!memcmp(ins, &insList[i], sizeof(ins_t))) return i;
	}
	return -1;
}

int SubYM2151::slot(ins_t *ins) {
	const int slotMap[8] = { 0x08,0x08,0x08,0x08,0x0c,0x0e,0x0e,0x0f };
	return slotMap[ins->ch.algo&7];
}

bool SubYM2151::compare(ins_t *ins1, ins_t *ins2, bool use_ext_cmp=false) {
	ym2151_op_t *op1, *op2;
	int i, sl;
	if (ins1->ch.algo!=ins2->ch.algo) return false;
	if (ins1->ch.fback!=ins2->ch.fback) return false;
	if (ins1->ch.noise!=ins2->ch.noise) return false;
	if (ins1->ch.nfreq!=ins2->ch.nfreq) return false;
	if (ins1->ch.lfreq!=ins2->ch.lfreq) return false;
	if (ins1->ch.lwave!=ins2->ch.lwave) return false;
	if (ins1->ch.pmd!=ins2->ch.pmd) return false;
	if (ins1->ch.amd!=ins2->ch.amd) return false;
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
		if (op1->dt2!=op2->dt2) return false;
		if (op1->ks!=op2->ks) return false;
		if (use_ext_cmp&&op1->am!=op2->am) return false;
	}
	return true;
}

void SubYM2151::write(int reg, int val, int f) {
	ym2151_op_t *op;
	//int dtTable[8]={0,1,2,3,0,-1,-2,-3};
	int ch;

	/**** early exits { ****/
	//if (reg<0x08) return;
	//if (reg==0xb3) return;
	//if ((reg&3)==3) return;
	/**** } ****/

	/**** OP ORDER: M1,M2,C1,C2 ****/
	//opnum = (reg&0x07)*4+((reg&0x18)>>3);	// 0-31
	ch = (reg&7);
	op = &insts[ch].ch.op[(reg&0x1F)>>3];
	switch (reg&0xe0) {
		case 0x00:	//
			if (reg==0x08) {	// KEY ON
				ch = val&7;
				if (val&0x78) {
					// KEY ON OP LO->HI: M1,C1,M2,C2
					if (memcmp(&instsPrev[ch],&insts[ch],sizeof(ins_t))) {
						if (find(&insts[ch])<0) add(&insts[ch],f);
					}
					memcpy(&instsPrev[ch],&insts[ch],sizeof(ins_t));
				}
			}
			else if (reg==0x0f)	{	//NE,NFREQ
				insts[ch].ch.noise = (val>>7)&1;
				insts[ch].ch.nfreq = val&0x1f;
			}
			else if (reg==0x18) {	//LFREQ
				insts[ch].ch.lfreq = val&0xff;
			}
			else if (reg==0x19) {	//PMD/AMD
				if (val&0x80) insts[ch].ch.pmd = val&0x7f;
				else insts[ch].ch.amd = val&0x7f;
			}
			else if (reg==0x1B) {	//LWAVE
				insts[ch].ch.lwave = val&0x3;
			}
			break;
		case 0x20:	//CH
			if (reg<0x28) {	//FB,ALGO
				insts[ch].ch.algo = val&0x7;
				insts[ch].ch.fback = (val>>3)&0x7;
			}
			else if (reg<0x30) {	//KC, IGNORE
			}
			else if (reg<0x38) {	//KF, IGNORE
			}
			else if (reg<0x40) {	//PMS,AMS
				insts[ch].ch.pms = (val>>4)&0x7;
				insts[ch].ch.ams = val&0x3;
			}
			break;
		case 0x40:	//DT1,MUL
			op->dt = (val>>4)&7;
			//op->detune = dtTable[op->dt];
			op->mul = val&0x0f;
			break;
		case 0x60:	//TL
			op->tl = val&0x7f;
			break;
		case 0x80:	//KS,AR
			op->ks = (val>>6)&3;
			op->ar = val&0x1f;
			break;
		case 0xA0:	//AM,D1R
			op->am = (val>>7)&1;
			op->dr = val&0x1f;
			break;
		case 0xC0:	//DT2,D2R
			op->dt2 = (val>>6)&3;
			op->sr = val&0x1f;
			break;
		case 0xE0:	//D1L,RR
			op->sl = val>>4;
			op->rr = val&0x0f;
			break;
		default: break;
	}
}

unsigned SubYM2151::group(bool use_ext_cmp=false) {
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
		//print("Grouping #",j,"...");
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
			//print("found ",id,"\n");
			for (i = 0; i<z; ++i) {
				if (insList[i].id==j) {
					if (i!=id) insList[i].id = -1;
				}
			}
		}
		else print("No master presets found for #",j," :(\n");
	}
	return u;
}

void SubYM2151::process(const lstring &fl, const V2PConfiguration &cfg) {
	string fn_edit;
	unsigned j,i;
	unsigned z = insList.size();
	unsigned used = group(cfg.use_ext_cmp_ym2151);
	print("OK: ",used," instrument(s) were found\n");
	for (j = 0; j<used; ++j) {
		//print("Looking for ins ",j,"...\n");
		for (i = 0; i<z; ++i) {
			//print("ins ",i," = ",insList[i].id,"\n");
			if (insList[i].id==j) {
				if (insList[i].file_id>fl.size()) {
					print("File ID ",insList[i].file_id," is out of bounds (",fl.size(),"), skipping...\n");
					continue;
				}
				fn_edit = {basename(fl[insList[i].file_id]),"_",j};
				//print("Saving ",notdir(fn_edit),"...");
				if (!save(insList[i], fn_edit, cfg)) {
					print("ERR: Can't save file '",fn_edit,"'\n");
					break;
				}
				//else print("done!\n");
			}
		}
	}
}

bool SubYM2151::save(const ins_t &ins, const string &fn, const V2PConfiguration &cfg) {
	unsigned ret = 0;
	if (cfg.dump_ym2151_opm) {++ret; if (!saveOPM(ins, fn)) --ret;}
	return ret>0;
}

string SubYM2151::toString(const ins_t &ins, unsigned num, const string &fn) {
	return (string)(
		"@:",num," ",fn,"\n",
		"LFO:",ins.ch.lfreq," ",ins.ch.amd," ",ins.ch.pmd," ",ins.ch.lwave," ",ins.ch.nfreq,"\n",
		"CH: 64 ",ins.ch.fback," ",ins.ch.algo," ",ins.ch.ams," ",ins.ch.pms," 120 ",ins.ch.noise?"1":"0","\n"
		"M1: ",ins.ch.op[0].ar," ",ins.ch.op[0].dr," ",ins.ch.op[0].sr," ",ins.ch.op[0].rr," ",ins.ch.op[0].sl," ",ins.ch.op[0].ks," ",ins.ch.op[0].mul," ",ins.ch.op[0].dt," ",ins.ch.op[0].dt2," ",ins.ch.op[0].am?"1":"0","\n",
		"C1: ",ins.ch.op[2].ar," ",ins.ch.op[2].dr," ",ins.ch.op[2].sr," ",ins.ch.op[2].rr," ",ins.ch.op[2].sl," ",ins.ch.op[2].ks," ",ins.ch.op[2].mul," ",ins.ch.op[2].dt," ",ins.ch.op[2].dt2," ",ins.ch.op[2].am?"1":"0","\n",
		"M2: ",ins.ch.op[1].ar," ",ins.ch.op[1].dr," ",ins.ch.op[1].sr," ",ins.ch.op[1].rr," ",ins.ch.op[1].sl," ",ins.ch.op[1].ks," ",ins.ch.op[1].mul," ",ins.ch.op[1].dt," ",ins.ch.op[1].dt2," ",ins.ch.op[1].am?"1":"0","\n",
		"C2: ",ins.ch.op[3].ar," ",ins.ch.op[3].dr," ",ins.ch.op[3].sr," ",ins.ch.op[3].rr," ",ins.ch.op[3].sl," ",ins.ch.op[3].ks," ",ins.ch.op[3].mul," ",ins.ch.op[3].dt," ",ins.ch.op[3].dt2," ",(ins.ch.op[3].am?"1":"0"),"\n"
	);
}

bool SubYM2151::saveOPM(const ins_t &ins, const string &fn) {
	string dataOPM;
	unsigned aa = 0;
	dataOPM = toString(ins, aa, fn);
	return false;
	//return file::write({fn,".opm"}, dataOPM, sizeof(dataOPM));
}

