#ifndef __VGM_GD3_H__
#define __VGM_GD3_H__

#include <nall/nall.hpp>
using namespace nall;

/****
GD3 v1.00
strings are null-terminated UCS2/UTF-16LE
****/
namespace VGM {
	namespace GD3 {
		struct Info {
			string track;
			string game;
			string system;
			string author;
		};
	}
	struct gd3_t {
		char id[4];	// "Gd3 "
		uint32_t version;	// BCD format
		uint32_t size;
		GD3::Info infoEn;
		GD3::Info infoJp;
		string date;
		string creator;
		string notes;
	};
}

#endif