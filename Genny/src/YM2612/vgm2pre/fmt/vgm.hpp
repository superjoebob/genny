#ifndef __VGM_H__
#define __VGM_H__

#include <nall/nall.hpp>
#include <nall/file.hpp>
#include <nall/gzip.hpp>

using namespace nall;

#include "vgm.command.hpp"
#include "vgm.types.hpp"
#include "vgm.header.hpp"
#include "vgm.gd3.hpp"

namespace VGM {
	/****
	struct vgm_t
		-[header_t hdr]: VGM header
		-[vector<data_t> data]: VGM data
		-[vector<uint8_t> dac]: sampled digital audio
		+[void load]: load VGM into struct
		+[void save]: save VGM to file
		+[void output]: output VGM to text
	****/
	struct vgm_t {
		header_t hdr;
		vector<data_t> data;
		vector<block_t> blocks;
		function<unsigned (const uint8_t *data, Command cmd)> processor;
		void load(const string &fn) {
			gzip f;
			if (f.decompress(fn)) {
				//print("Compressed ");
				load(f.data, f.size);
			}
			else {
				//print("Uncompressed ");
				auto mem = file::read(fn);
				load(mem.data(), mem.size());
			}
		}
		void load(uint8_t *data, unsigned size) {
			// TODO: load basic header
			// branch depending on hdr.version
			if (loadHeader(hdr, data)) {
				uint8_t d[4];
				unsigned data_start = hdr.version<0x150?0x40:Addr::Data+hdr.offset_data;
				unsigned p = data_start, q;
				print("VGM v",hex((hdr.version>>8)&0xff),".",hex(hdr.version&0xff));
				//print(", data start x",hex(data_start));
				while ((Command)data[p]!=DATA_END&&p<size) {
					Command c = (Command)data[p++];
					p += processor(data+p,c);
				}
			}
			else print("VGM check failed");
		}
		void save(file &f) {
		}
		void output(file &f) {
		}
	};
}

#endif