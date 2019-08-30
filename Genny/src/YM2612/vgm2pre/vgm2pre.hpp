#ifndef __VGM2_H__
#define __VGM2_H__

#include <nall/nall.hpp>
using namespace nall;

#include <phoenix/phoenix.hpp>
using namespace phoenix;

#include "fmt/vgm.hpp"
#include "ui/win.about.hpp"
#include "ui/win.opt.hpp"
#include "config.hpp"
#include "sub.ym2612.hpp"

struct Processor {
	VGM::vgm_t vgm;
	uint32_t _T_WAIT[2];
	lstring filelist;
	unsigned smp;
	unsigned file_id;
#ifdef __SUB_YM2612_H__
	SubYM2612 sub2612;
#endif
#ifdef __SUB_YM2151_H__
	//SubYM2151 sub2151;
#endif
	Processor() {
		init();
		vgm.processor = [&](const uint8_t *data, VGM::Command cmd) {
			// NOTE: use of memcpy here assumes little endian
			uint16_t n;
			uint32_t d;
			uint8_t q[4];
			unsigned ret = 0;
			//print(VGM::Commands[cmd]);
			switch (cmd) {
				case VGM::DATA_END:
					//print("==End\n");
					break;
				case VGM::DATA_START:
					//print("==Data");
					q[0] = data[0]; q[1] = data[1];
					if ((VGM::Command)q[0]==VGM::DATA_END) {
						//memcpy(&d,data+2,4);
						d = data[2]+(data[3]<<8)+(data[4]<<16)+(data[5]<<24);
						//print(", type ",VGM::Blocks[q[1]]," length ",d);
						ret = 2+4+d;
					}
					//else print(", Invalid data block");
					//print("\n");
					break;
				case VGM::WAIT_NTSC:
				case VGM::WAIT_PAL:
					//print("Wait ",(cmd-VGM::WAIT_NTSC?882:735),"\n");
					smp += _T_WAIT[cmd-VGM::WAIT_NTSC];
					break;
				case VGM::WAIT_16BIT:
					memcpy(&n,data,2);
					smp += n;
					//print("Wait ",n,"\n");
					ret = 2;
					break;
				case VGM::WAIT_01:
				case VGM::WAIT_02:
				case VGM::WAIT_03:
				case VGM::WAIT_04:
				case VGM::WAIT_05:
				case VGM::WAIT_06:
				case VGM::WAIT_07:
				case VGM::WAIT_08:
				case VGM::WAIT_09:
				case VGM::WAIT_0A:
				case VGM::WAIT_0B:
				case VGM::WAIT_0C:
				case VGM::WAIT_0D:
				case VGM::WAIT_0E:
				case VGM::WAIT_0F:
				case VGM::WAIT_10:
					q[0] = 1+(cmd&0xf);
					smp += q[0];
					break;
				case VGM::OVERRIDE_WAIT_LENGTH:
					q[0] = data[0];
					memcpy(&n,data+1,2);
					_T_WAIT[q[0]-VGM::WAIT_NTSC] = n;
					ret = 3;
					break;
				case VGM::SEEK:
					ret = 4;
					break;
				case VGM::WRITE_PSG_STEREO:
				case VGM::WRITE_PSG:
					//print("PSG write x",hex(cmd)," x",hex(*data),"\n");
					ret = 1;
					break;
				case VGM::WRITE_YM2413:
					q[0] = data[0]; q[1] = data[1];
					//print("YM2413 write x",hex(q[0])," x",hex(q[1]),"\n");
					ret = 2;
					break;
				case VGM::WRITE_YM2612_1:
				case VGM::WRITE_YM2612_2:
					q[0] = data[0]; q[1] = data[1];
#ifdef __SUB_YM2612_H__
					sub2612.write(
						cmd-VGM::WRITE_YM2612_1,
						q[0],
						q[1],
						file_id
					);
#endif
					//print("YM2612[",(cmd-VGM::WRITE_YM2612_1),"] write x",hex(q[0])," x",hex(q[1]),"\n");
					ret = 2;
					break;
				case VGM::WRITE_YM2612_DAC_WAIT_0:
				case VGM::WRITE_YM2612_DAC_WAIT_1:
				case VGM::WRITE_YM2612_DAC_WAIT_2:
				case VGM::WRITE_YM2612_DAC_WAIT_3:
				case VGM::WRITE_YM2612_DAC_WAIT_4:
				case VGM::WRITE_YM2612_DAC_WAIT_5:
				case VGM::WRITE_YM2612_DAC_WAIT_6:
				case VGM::WRITE_YM2612_DAC_WAIT_7:
				case VGM::WRITE_YM2612_DAC_WAIT_8:
				case VGM::WRITE_YM2612_DAC_WAIT_9:
				case VGM::WRITE_YM2612_DAC_WAIT_A:
				case VGM::WRITE_YM2612_DAC_WAIT_B:
				case VGM::WRITE_YM2612_DAC_WAIT_C:
				case VGM::WRITE_YM2612_DAC_WAIT_D:
				case VGM::WRITE_YM2612_DAC_WAIT_E:
				case VGM::WRITE_YM2612_DAC_WAIT_F:
					break;
				case VGM::WRITE_YM2151:
					q[0] = data[0]; q[1] = data[1];
					// TODO: DUMP TO OPM
#ifdef __SUB_YM2151_H__
					/*sub2151.write(
						q[0],
						q[1],
						file_id
					);*/
#endif
					//print("YM2151 write x",hex(q[0])," x",hex(q[1]),"\n");
					ret = 2;
					break;
				case VGM::WRITE_OKIM6258:
					q[0] = data[0]; q[1] = data[1];
					//print("OKIM6295 write x",hex(q[0])," x",hex(q[1]),"\n");
					ret = 2;
					break;
				case VGM::WRITE_OKIM6295:
					q[0] = data[0]; q[1] = data[1];
					//print("OKIM6295 write x",hex(q[0])," x",hex(q[1]),"\n");
					ret = 2;
					break;
				case VGM::WRITE_YM2203:
				case VGM::WRITE_YM2608_1:
				case VGM::WRITE_YM2608_2:
				case VGM::WRITE_YM2610_1:
				case VGM::WRITE_YM2610_2:
				case VGM::WRITE_YM3812:
				case VGM::WRITE_YM3526:
				case VGM::WRITE_Y8950:
				case VGM::WRITE_YMZ280B:
				case VGM::WRITE_YMF262_1:
				case VGM::WRITE_YMF262_2:
				case VGM::_UNUSED_x60:
					ret = 2;
					break;
				case VGM::WRITE_AY8910:
				case VGM::WRITE_RF5C68:
				case VGM::WRITE_RF5C164:
				case VGM::WRITE_PWM:
				case VGM::WRITE_DMG:
				case VGM::WRITE_NES:
				case VGM::WRITE_MPCM:
				case VGM::WRITE_UPD7759:
				case VGM::WRITE_HUC6280:
				case VGM::WRITE_K053260:
				case VGM::WRITE_POKEY:
				case VGM::_UNUSED_xBC:
				case VGM::_UNUSED_xBD:
				case VGM::_UNUSED_xBE:
				case VGM::_UNUSED_xBF:
					ret = 2;
					break;
				case VGM::WRITE_SPCM:
				case VGM::WRITE_RF5C68_MEM:
				case VGM::WRITE_RF5C164_MEM:
				case VGM::WRITE_MPCM_BANK:
				case VGM::WRITE_QSOUND:
				case VGM::WRITE_YMF278B:
				case VGM::WRITE_YMF271:
				case VGM::WRITE_SCC1:
				case VGM::WRITE_K054539:
				case VGM::WRITE_C140:
					ret = 3;
					break;
				default:
					print("\nUnsupported VGM command x",hex(cmd),"\n");
					break;
			}
			return ret;
		};
	}
	bool finalize();
	void process(const string &fn) {
		reset();
#ifdef __SUB_YM2612_H__
		sub2612.init();
#endif
#ifdef __SUB_YM2151_H__
		//sub2151.init();
#endif
		print("OK: Processing '",fn,"' (");
		vgm.load(fn);
		print(")\n");
		++file_id;
		//print("Samples processed: ",smp,"\n");
	}
	void processFile(const string &fn) {
		if (file::exists(fn)) {
			filelist.append(fn);
			process(fn);
		}
	}
	void processDirectory(const string &d) {
		//filelist.empty();
		string fn;
		if (directory::exists(d)) {
			lstring vgms = directory::contents(d, "*.vg*");
			if (vgms.size()==0) print("ERR: No .vgm files in directory or some other fatal error\n");
			else {
				for (auto &v : vgms) {
					if (v.iendsWith(".vgm")||v.iendsWith(".vgz")||v.iendsWith(".vgm.gz")) {
						fn = (string){d,!d.endsWith("/")&&!d.endsWith("\\")?"/":"",v};
						processFile(fn);
					}
				}
			}
		}
		else print(d," is not a directory!\n");
	}
	void reset() {
		_T_WAIT[0] = VGM::Wait::NTSC;
		_T_WAIT[1] = VGM::Wait::PAL;
		smp = 0;
	}
	void init() {
		reset();
		filelist.reset();
		file_id = 0;
	}
};

struct Prog {
	/** program properties **/
	string name;	/* app name */
	float version;	/* app version */
	V2PConfiguration cfg;	/* app config */
	Processor proc;
	/** program methods **/
	void open(const lstring &files);	/* open a file */
	void close();	/* close a file */
	void interactiveMode(const lstring &files);	/* open the GUI */
	void usage();	/* print out program usage */
	void init();	/* initialize program variables */
	/** constructors **/
	Prog(int argc, char **argv);
} *prog = nullptr;

bool Processor::finalize() {
	if (filelist.size()>0) {
#ifdef __SUB_YM2612_H__
		sub2612.process(filelist,prog->cfg);
#endif
#ifdef __SUB_YM2151_H__
		//sub2151.process(filelist,prog->cfg);
#endif
		return true;
	}
	else return false;
}


struct Win : Window {
	/**
	 * "Fake" status bar class
	 **/
	struct StatusLayout : HorizontalLayout {
		uint8_t prgVal;
		ProgressBar prg;
		Label status;
		Button bDo;
		StatusLayout() : HorizontalLayout() {
			init();
			reflow();
		}
		void reflow() {
			append(prg, {100, 12}, 4);
			append(status, {~0, 12}, 4);
			append(bDo, {0, ~0});
		}
		void init() {
			setMargin(2);
			setProgress(0);
			setStatus("Ready.");
			bDo.setText("Dump");
			bDo.setEnabled(false);
			bDo.onActivate = [&]() {
				if (prog->proc.finalize()) {
					setStatus(string{
						"Dumping complete.",
						!prog->cfg.disabledOPN[N::OPN::Types::GYB]&&prog->cfg.dumpOPN[N::OPN::Types::GYB]?
							" GYB was selected, check program's folder":
							"",
						!prog->cfg.disabledOPM[N::OPM::Types::OPM]&&prog->cfg.dumpOPM[N::OPM::Types::OPM]?
							" OPM was selected, check program's folder":
							"",
					});
				}
				else {
					setStatus("Dumping failed.");
				}
			};
		}
		void setProgress(uint8_t v) {
			prgVal = v<100?v:100;
			prg.setPosition(prgVal);
		}
		void setStatus(const string &s) {
			status.setText(s);
		}
		/*Timer tmPrg;
		//Window parentWin;
		void create(const string &s="Done.") {
			setMargin(2);
			status.setText(s);
			tmPrg.onActivate = [&]() {
				if (prgVal<101) {
					prg.setPosition(prgVal);
					status.setText({"(",prgVal,")"});
					++prgVal;
				}
				else {
					tmPrg.setEnabled(false);
					MessageWindow()
						.setTitle({"Drop failed - ", prog->name})
						.setText({
							"Done!"
						})
						.information();
				}
			};
			reflow();
		}
		void start(uint8_t st=0) {
			prgVal = st;
			tmPrg.setEnabled(true);
			tmPrg.setInterval(50);
		}
		void reset() {
			prgVal = 0;
			tmPrg.setEnabled(true);
		}*/
	};
	/** menus **/
	Menu mFile;
		Item mFileOpen;
		Item mFileOpenDir;
		Item mFileQuit;
	Menu mTools;
		Item mToolsPrefs;
	Menu mHelp;
		Item mHelpAbout;
	/** windows **/
	WinAbout wAbt;
	WinOptions wOpt;
	/** layouts **/
	VerticalLayout lMain;
	HorizontalLayout lLog;
/*	VerticalLayout lTab1, lTab1a, lTab2, lTab3;
	HorizontalLayout lT2Btns;
	TabFrame lTabbed;
	Frame fsT1;
*/	/** widgets **/
	TextEdit out;
	ListView lvFiles;
	StatusLayout bar;
/*	FileEntry feFile;
	Label lblText, lblT2, lblT3;
	Button bImg, bImgText, bText;
	NCanvas cnSmp;
	unsigned clicks;
*/	/** aux **/
//	Spritesheet sprTabs;
	BrowserWindow browser;	/* file browser for "Open File" functionality */
/*	NTerra::device_t dev;
	NTerra::mesh_t mesh;
	Curve bzc;
	Timer tUpdater, tRenderer;
*/	/** proc **/
	void openFile(const lstring &files);
	void init();
	void initProc();
	void reflowStatic();
	void reflow();
	void resize();
	/**** more proc ****/
	void repaint();
	void reupdate();
	/** constructors **/
	Win();
} *win = nullptr;

#endif