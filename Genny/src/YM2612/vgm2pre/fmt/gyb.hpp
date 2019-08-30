#ifndef __GYB_H__
#define __GYB_H__

#include <nall/nall.hpp>
#include <nall/file.hpp>
#include <nall/gzip.hpp>

using namespace nall;

const int ID_GYB = 26*0x01|12*0x100;

struct gyb_ins_t {
	string name;
	unsigned char reg[0x20];
};

struct gyb_bank_t {
	uint8_t count;	// UNNEEDED WITH VECTOR
	uint8_t instMap[128];
	vector<gyb_ins_t> insts;
	gyb_bank_t() {for (unsigned i=0; i<128; ++i) instMap[i] = 255;}
	void raw(uint8_t *data, unsigned start) {}
};

struct gyb_file_t {
	uint8_t version;
	uint8_t lfo;
	gyb_bank_t melody;
	gyb_bank_t drums;
	static uint32_t calc_checksum(uint32_t fs, const uint8_t *data) {
		uint32_t pos, sum, tmp;
		uint8_t chk1, chk2, chk3, mask;
		uint16_t count;
		uint8_t chk[4];
		/**** nineko's crazy checksum formula ****/
		// chk2
		chk2 = 0;
		tmp = fs; while (tmp) {
			chk2 += (tmp%10);
			tmp /= 10;
		}
		chk2 *= 3;
		// chk3
		chk3 = 0;
		sum = 0;
		mask = 1<<(fs&0x07);
		for (pos = 0x00; pos<fs; ++pos) {
			if ((data[pos]&mask)==mask) ++chk3;
			sum += data[pos];
		}
		// chk1
		count = data[0x03]+data[0x04];
		chk1 = (fs+sum)%(count+1);
		sum %= 999;
		// do it
		chk[0x00] = chk2+(sum%37);
		chk[0x01] = chk1;
		chk[0x02] = chk3;
		// THE CRAZY
		chk[0x03] = (
			sum*(chk1+1)*(chk2+2)*(chk3+3)+
			sum*(chk1+1)*(chk2+2)+
			sum*(chk2+2)*(chk3+3)+
			sum*(chk1+1)*(chk3+3)+
			chk1+chk2+chk3+84
		)%199;
		memcpy(&tmp, chk, 0x04);
		//return 0;
		return tmp;
	}
};

#endif