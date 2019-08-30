#ifndef __VGM_HDR_H__
#define __VGM_HDR_H__

#include <nall/nall.hpp>

using namespace nall;

#include "vgm.types.hpp"

namespace VGM {
	struct HeaderCommon {
		char id[4];	// "Vgm "
		uint32_t offset_eof;
		uint32_t version;
		uint32_t clock_sn76489;
		uint32_t clock_ym2413;
		uint32_t offset_gd3;
		uint32_t samples_total;
		uint32_t offset_loop;
		uint32_t samples_loop;
		uint32_t rate;
		uint16_t sn_feedback;	// <1.10 = reserved >= this offset
		uint8_t sn_width;
		uint8_t sn_flags;	// <1.51 = reserved 0
		uint32_t clock_ym2612;
		uint32_t clock_ym2151;
		uint32_t offset_data;
		uint32_t clock_spcm;	// <1.51 = reserved 0
		uint32_t register_spcm;	// <1.51 = reserved 0
	};
	struct Header151a : HeaderCommon {
		uint32_t clock_ym3812;
		uint32_t _reserved_44;
		uint32_t _reserved_48;
		uint32_t _reserved_4C;
	};
	struct Header151 : HeaderCommon {
		uint32_t clock_rf5c68;	// instead of ym3812
		uint32_t clock_ym2203;
		uint32_t clock_ym2608;
		uint32_t clock_ym2610;
		uint32_t clock_ym3812;
		uint32_t clock_ym3526;
		uint32_t clock_y8950;
		uint32_t clock_ymf262;
		uint32_t clock_ymf278b;
		uint32_t clock_ymf271;
		uint32_t clock_ymz280b;
		uint32_t clock_rf5c164;
		uint32_t clock_pwm;
		uint32_t clock_ay8910;
		uint8_t type_ay;
		uint8_t flags_ay;
		uint8_t flags_ay_ym2203;
		uint8_t flags_ay_ym2608;
		uint8_t mod_volume;	// <1.60 = reserved
		uint8_t _reserved_7D;
		uint8_t min_loop;	// <1.60 = reserved
		uint8_t mod_loop;
	};
	struct Header160 : Header151 {
	};
	struct Header161 : Header160 {
		uint32_t clock_dmg;
		uint32_t clock_nes;
		uint32_t clock_mpcm;
		uint32_t clock_upd7759;
		uint32_t clock_okim6258;
		uint8_t flags_okim6258;
		uint8_t flags_k054539;
		uint8_t flags_c140;
		uint8_t _reserved_97;
		uint32_t clock_okim6295;
		uint32_t clock_k051649;
		uint32_t clock_k054539;
		uint32_t clock_huc6280;
		uint32_t clock_c140;
		uint32_t clock_k053260;
		uint32_t clock_pokey;
		uint32_t clock_qsound;
		uint32_t _reserved_B8;
		uint32_t offset_extra;	// <1.70 = reserved
	};
	struct Header170 : Header161 {
		uint8_t _reserved_C0[0x40];
	};
	struct header_t : Header170 {};	// always the latest
	struct HeaderExtra {
		uint32_t size;
		uint32_t offset_clock;
		uint32_t offset_volume;
	};
	struct HeaderExtraClock {
		uint8_t type;
		uint32_t data;
	};
	struct HeaderExtraVolume {
		uint8_t type;
		uint16_t data;
	};
	bool loadHeader(header_t &h, uint8_t *d) {
		memcpy(&h, d, sizeof(header_t));
		//print(memcmp(h.id,ID::VGM,4),"\n");
		if (memcmp(h.id,ID::VGM,4)!=0) return false;
		else {
			return true;
		}
	}
}

#endif