#ifndef __VGM_CMD_H__
#define __VGM_CMD_H__

/****
VGM file version applies to allowance of VGM::Command values
****/
namespace VGM {
	enum Command : uint8_t {
		// command + 1 byte
		WRITE_PSG_STEREO = 0x4f,
		WRITE_PSG,
		// command + 2 bytes
		WRITE_YM2413,
		WRITE_YM2612_1,
		WRITE_YM2612_2,
		WRITE_YM2151,
		WRITE_YM2203,
		WRITE_YM2608_1,
		WRITE_YM2608_2,
		WRITE_YM2610_1,
		WRITE_YM2610_2,
		WRITE_YM3812,
		WRITE_YM3526,
		WRITE_Y8950,
		WRITE_YMZ280B,
		WRITE_YMF262_1,
		WRITE_YMF262_2,
		_UNUSED_x60,
		// special
		WAIT_16BIT,
		WAIT_NTSC,
		WAIT_PAL,
		OVERRIDE_WAIT_LENGTH,
		_UNUSED_x65,
		DATA_END,
		DATA_START,
		WRITE_PCM,
		_UNUSED_x69,
		_UNUSED_x6A,
		_UNUSED_x6B,
		_UNUSED_x6C,
		_UNUSED_x6D,
		_UNUSED_x6E,
		_UNUSED_x6F,
		// wait n+1 samples
		WAIT_01,
		WAIT_02,
		WAIT_03,
		WAIT_04,
		WAIT_05,
		WAIT_06,
		WAIT_07,
		WAIT_08,
		WAIT_09,
		WAIT_0A,
		WAIT_0B,
		WAIT_0C,
		WAIT_0D,
		WAIT_0E,
		WAIT_0F,
		WAIT_10,
		// ym2612 dac write + wait
		WRITE_YM2612_DAC_WAIT_0,
		WRITE_YM2612_DAC_WAIT_1,
		WRITE_YM2612_DAC_WAIT_2,
		WRITE_YM2612_DAC_WAIT_3,
		WRITE_YM2612_DAC_WAIT_4,
		WRITE_YM2612_DAC_WAIT_5,
		WRITE_YM2612_DAC_WAIT_6,
		WRITE_YM2612_DAC_WAIT_7,
		WRITE_YM2612_DAC_WAIT_8,
		WRITE_YM2612_DAC_WAIT_9,
		WRITE_YM2612_DAC_WAIT_A,
		WRITE_YM2612_DAC_WAIT_B,
		WRITE_YM2612_DAC_WAIT_C,
		WRITE_YM2612_DAC_WAIT_D,
		WRITE_YM2612_DAC_WAIT_E,
		WRITE_YM2612_DAC_WAIT_F,
		// dac stream control
		WRITE_DAC_STREAM_SETUP,
		WRITE_DAC_STREAM_DATA,
		WRITE_DAC_STREAM_FREQ,
		WRITE_DAC_STREAM_START,
		WRITE_DAC_STREAM_STOP,
		WRITE_DAC_STREAM_START_FAST,
		// command + 2 bytes read
		WRITE_AY8910 = 0xa0,
		WRITE_RF5C68 = 0xb0,
		WRITE_RF5C164,
		WRITE_PWM,
		WRITE_DMG,
		WRITE_NES,
		WRITE_MPCM,
		WRITE_UPD7759,
		WRITE_OKIM6258,
		WRITE_OKIM6295,
		WRITE_HUC6280,
		WRITE_K053260,
		WRITE_POKEY,
		_UNUSED_xBC,
		_UNUSED_xBD,
		_UNUSED_xBE,
		_UNUSED_xBF,
		// command + 3 bytes read
		WRITE_SPCM,
		WRITE_RF5C68_MEM,
		WRITE_RF5C164_MEM,
		WRITE_MPCM_BANK,
		WRITE_QSOUND,
		WRITE_YMF278B = 0xd0,
		WRITE_YMF271,
		WRITE_SCC1,
		WRITE_K054539,
		WRITE_C140,
		// command + 4 bytes read
		SEEK = 0xe0
	};
	const string Commands[] = {
		"INVALID::00", "INVALID::01", "INVALID::02", "INVALID::03",
		"INVALID::04", "INVALID::05", "INVALID::06", "INVALID::07",
		"INVALID::08", "INVALID::09", "INVALID::0A", "INVALID::0B",
		"INVALID::0C", "INVALID::0D", "INVALID::0E", "INVALID::0F",
		"INVALID::10", "INVALID::11", "INVALID::12", "INVALID::13",
		"INVALID::14", "INVALID::15", "INVALID::16", "INVALID::17",
		"INVALID::18", "INVALID::19", "INVALID::1A", "INVALID::1B",
		"INVALID::1C", "INVALID::1D", "INVALID::1E", "INVALID::1F",
		"INVALID::20", "INVALID::21", "INVALID::22", "INVALID::23",
		"INVALID::24", "INVALID::25", "INVALID::26", "INVALID::27",
		"INVALID::28", "INVALID::29", "INVALID::2A", "INVALID::2B",
		"INVALID::2C", "INVALID::2D", "INVALID::2E", "INVALID::2F",
		"RESERVED::30", "RESERVED::31", "RESERVED::32", "RESERVED::33",
		"RESERVED::34", "RESERVED::35", "RESERVED::36", "RESERVED::37",
		"RESERVED::38", "RESERVED::39", "RESERVED::3A", "RESERVED::3B",
		"RESERVED::3C", "RESERVED::3D", "RESERVED::3E", "RESERVED::3F",
		"RESERVED::40", "RESERVED::41", "RESERVED::42", "RESERVED::43",
		"RESERVED::44", "RESERVED::45", "RESERVED::46", "RESERVED::47",
		"RESERVED::48", "RESERVED::49", "RESERVED::4A", "RESERVED::4B",
		"RESERVED::4C", "RESERVED::4D", "RESERVED::4E",
		"PSG::write stereo",
		"PSG::write",
		"YM2413::write",
		"YM2612[P1]::write",
		"YM2612[P2]::write",
		"YM2151::write",
		"YM2203::write",
		"YM2608[P1]::write",
		"YM2608[P2]::write",
		"YM2610[P1]::write",
		"YM2610[P2]::write",
		"YM3812::write",
		"YM3526::write",
		"Y8950::write",
		"YMZ280B::write",
		"YMF262[P1]::write",
		"YMF262[P2]::write",
		"RESERVED::60",
		// special
		"WAIT::16BIT",
		"WAIT::NTSC",
		"WAIT::PAL",
		"WAIT::overrideLength",
		"RESERVED::65",
		"DATA::end",
		"DATA::start",
		"SegaPCM::writeRAM",
		"RESERVED::69", "RESERVED::6A", "RESERVED::6B",
		"RESERVED::6C", "RESERVED::6D", "RESERVED::6E", "RESERVED::6F",
		// wait n+1 samples
		"WAIT::01", "WAIT::02", "WAIT::03", "WAIT::04",
		"WAIT::05", "WAIT::06", "WAIT::07", "WAIT::08",
		"WAIT::09", "WAIT::0A", "WAIT::0B", "WAIT::0C",
		"WAIT::0D", "WAIT::0E", "WAIT::0F", "WAIT::10",
		// ym2612 dac write + wait
		"YM2612[DAC]::writeWait(0)",
		"YM2612[DAC]::writeWait(1)",
		"YM2612[DAC]::writeWait(2)",
		"YM2612[DAC]::writeWait(3)",
		"YM2612[DAC]::writeWait(4)",
		"YM2612[DAC]::writeWait(5)",
		"YM2612[DAC]::writeWait(6)",
		"YM2612[DAC]::writeWait(7)",
		"YM2612[DAC]::writeWait(8)",
		"YM2612[DAC]::writeWait(9)",
		"YM2612[DAC]::writeWait(A)",
		"YM2612[DAC]::writeWait(B)",
		"YM2612[DAC]::writeWait(C)",
		"YM2612[DAC]::writeWait(D)",
		"YM2612[DAC]::writeWait(E)",
		"YM2612[DAC]::writeWait(F)",
		// dac stream control
		"DAC_STREAM::setup",
		"DAC_STREAM::setData",
		"DAC_STREAM::setFreq",
		"DAC_STREAM::start",
		"DAC_STREAM::stop",
		"DAC_STREAM::startFast",
		"RESERVED::96", "RESERVED::97",
		"RESERVED::98", "RESERVED::99", "RESERVED::9A", "RESERVED::9B",
		"RESERVED::9C", "RESERVED::9D", "RESERVED::9E", "RESERVED::9F",
		// command + 2 bytes read
		"AY8910::write",
		"RESERVED::A1", "RESERVED::A2", "RESERVED::A3",
		"RESERVED::A4", "RESERVED::A5", "RESERVED::A6", "RESERVED::A7",
		"RESERVED::A8", "RESERVED::A9", "RESERVED::AA", "RESERVED::AB",
		"RESERVED::AC", "RESERVED::AD", "RESERVED::AE", "RESERVED::AF",
		"RF5C68::write",
		"RESERVED::B1", "RESERVED::B2", "RESERVED::B3",
		"RESERVED::B4", "RESERVED::B5", "RESERVED::B6", "RESERVED::B7",
		"RESERVED::B8", "RESERVED::B9", "RESERVED::BA", "RESERVED::BB",
		"RF5C164::write",
		"PWM::write",
		"DMG::write",
		"NES::write",
		"MultiPCM::write",
		"uPD7759::write",
		"OKIM6258::write",
		"OKIM6295::write",
		"HuC6280::write",
		"K053260::write",
		"POKEY::write",
		"RESERVED::BC", "RESERVED::BD", "RESERVED::BE", "RESERVED::BF",
		// command + 3 bytes read
		"SegaPCM::writeMem",
		"RF5C68::writeMem",
		"RF5C164::writeMem",
		"MultiPCM::setBankOffset",
		"QSound::write",
		"RESERVED::C5", "RESERVED::C6", "RESERVED::C7",
		"RESERVED::C8", "RESERVED::C9", "RESERVED::CA", "RESERVED::CB",
		"RESERVED::CC", "RESERVED::CD", "RESERVED::CE", "RESERVED::CF",
		"YMF278B::write",
		"YMF271::write",
		"SCC1::write",
		"K054539::write",
		"C140::write",
		"RESERVED::D5", "RESERVED::D6", "RESERVED::D7",
		"RESERVED::D8", "RESERVED::D9", "RESERVED::DA", "RESERVED::DB",
		"RESERVED::DC", "RESERVED::DD", "RESERVED::DE", "RESERVED::DF",
		// command + 4 bytes read
		"SEEK",
		// reserved
		"RESERVED::E1", "RESERVED::E2", "RESERVED::E3",
		"RESERVED::E4", "RESERVED::A5", "RESERVED::A6", "RESERVED::A7",
		"RESERVED::E8", "RESERVED::E9", "RESERVED::EA", "RESERVED::EB",
		"RESERVED::EC", "RESERVED::ED", "RESERVED::EE", "RESERVED::EF",
		"RESERVED::F0", "RESERVED::F1", "RESERVED::F2", "RESERVED::F3",
		"RESERVED::F4", "RESERVED::F5", "RESERVED::F6", "RESERVED::F7",
		"RESERVED::F8", "RESERVED::F9", "RESERVED::FA", "RESERVED::FB",
		"RESERVED::FC", "RESERVED::FD", "RESERVED::FE", "RESERVED::FF"
	};
	enum Block : uint8_t {
		// $00..$3E = uncompressed recorded stream
		U_PCM_YM2612 = 0,
		U_PCM_RF5C68,
		U_PCM_RF5C164,
		U_PCM_PWM,
		// $40..$7E = compressed recorded stream
		C_PCM_YM2612 = 0x40,
		C_PCM_RF5C68,
		C_PCM_RF5C164,
		C_PCM_PWM,
		// $80..$BF = ROM/RAM image dumps
		ROM_SPCM = 0x80,
		ROM_YM2608_DT,
		ROM_YM2610_ADPCM,
		ROM_YM2610_DT,
		ROM_YMF278B,
		ROM_YMF271,
		ROM_YMZ280B,
		RAM_YMF278B,
		ROM_Y8950_DT,
		ROM_MPCM,
		ROM_UPD7759,
		ROM_OKIM6295,
		ROM_K054539,
		ROM_C140,
		ROM_K053260,
		ROM_QSOUND,
		// $C0..$FF = direct RAM writes
		RAM_RF5C68 = 0xc0,
		RAM_RF5C164,
		RAM_NES
	};
	const string Blocks[] = {
		"PCM::YM2612",
		"PCM::RF5C68",
		"PCM::RF5C164",
		"PCM::PWM",
		"RESERVED::04", "RESERVED::05", "RESERVED::06", "RESERVED::07",
		"RESERVED::08", "RESERVED::09", "RESERVED::0A", "RESERVED::0B",
		"RESERVED::0C", "RESERVED::0D", "RESERVED::0E", "RESERVED::0F",
		"RESERVED::10", "RESERVED::11", "RESERVED::12", "RESERVED::13",
		"RESERVED::14", "RESERVED::15", "RESERVED::16", "RESERVED::17",
		"RESERVED::18", "RESERVED::19", "RESERVED::1A", "RESERVED::1B",
		"RESERVED::1C", "RESERVED::1D", "RESERVED::1E", "RESERVED::1F",
		"RESERVED::20", "RESERVED::21", "RESERVED::22", "RESERVED::23",
		"RESERVED::24", "RESERVED::25", "RESERVED::26", "RESERVED::27",
		"RESERVED::28", "RESERVED::29", "RESERVED::2A", "RESERVED::2B",
		"RESERVED::2C", "RESERVED::2D", "RESERVED::2E", "RESERVED::2F",
		"RESERVED::30", "RESERVED::31", "RESERVED::32", "RESERVED::33",
		"RESERVED::34", "RESERVED::35", "RESERVED::36", "RESERVED::37",
		"RESERVED::38", "RESERVED::39", "RESERVED::3A", "RESERVED::3B",
		"RESERVED::3C", "RESERVED::3D", "RESERVED::3E", "RESERVED::3F",
		"CPCM::YM2612",
		"CPCM::RF5C68",
		"CPCM::RF5C164",
		"CPCM::PWM",
		"RESERVED::44", "RESERVED::45", "RESERVED::46", "RESERVED::47",
		"RESERVED::48", "RESERVED::49", "RESERVED::4A", "RESERVED::4B",
		"RESERVED::4C", "RESERVED::4D", "RESERVED::4E", "RESERVED::4F",
		"RESERVED::50", "RESERVED::51", "RESERVED::52", "RESERVED::53",
		"RESERVED::54", "RESERVED::55", "RESERVED::56", "RESERVED::57",
		"RESERVED::58", "RESERVED::59", "RESERVED::5A", "RESERVED::5B",
		"RESERVED::5C", "RESERVED::5D", "RESERVED::5E", "RESERVED::5F",
		"RESERVED::60", "RESERVED::61", "RESERVED::62", "RESERVED::63",
		"RESERVED::64", "RESERVED::65", "RESERVED::66", "RESERVED::67",
		"RESERVED::68", "RESERVED::69", "RESERVED::6A", "RESERVED::6B",
		"RESERVED::6C", "RESERVED::6D", "RESERVED::6E", "RESERVED::6F",
		"RESERVED::70", "RESERVED::71", "RESERVED::72", "RESERVED::73",
		"RESERVED::74", "RESERVED::75", "RESERVED::76", "RESERVED::77",
		"RESERVED::78", "RESERVED::79", "RESERVED::7A", "RESERVED::7B",
		"RESERVED::7C", "RESERVED::7D", "RESERVED::7E",
		"CMPTABLE",
		"ROM::SegaPCM",
		"ROM::YM2608::DELTA-T",
		"ROM::YM2610::ADPCM",
		"ROM::YM2610::DELTA-T",
		"ROM::YMF278B",
		"ROM::YMF271",
		"ROM::YMZ280B",
		"RAM::YMF278B",
		"ROM::Y8950::DELTA-T",
		"ROM::MultiPCM",
		"ROM::UPD7759",
		"ROM::OKIM6295",
		"ROM::K054539",
		"ROM::C140",
		"ROM::K053260",
		"ROM::QSOUND",
		"RESERVED::90", "RESERVED::91", "RESERVED::92", "RESERVED::93",
		"RESERVED::94", "RESERVED::95", "RESERVED::96", "RESERVED::97",
		"RESERVED::98", "RESERVED::99", "RESERVED::9A", "RESERVED::9B",
		"RESERVED::9C", "RESERVED::9D", "RESERVED::9E", "RESERVED::9F",
		"RESERVED::A0", "RESERVED::A1", "RESERVED::A2", "RESERVED::A3",
		"RESERVED::A4", "RESERVED::A5", "RESERVED::A6", "RESERVED::A7",
		"RESERVED::A8", "RESERVED::A9", "RESERVED::AA", "RESERVED::AB",
		"RESERVED::AC", "RESERVED::AD", "RESERVED::AE", "RESERVED::AF",
		"RESERVED::B0", "RESERVED::B1", "RESERVED::B2", "RESERVED::B3",
		"RESERVED::B4", "RESERVED::B5", "RESERVED::B6", "RESERVED::B7",
		"RESERVED::B8", "RESERVED::B9", "RESERVED::BA", "RESERVED::BB",
		"RESERVED::BC", "RESERVED::BD", "RESERVED::BE", "RESERVED::BF",
		"RAM::RF5C68",
		"RAM::RF5C164",
		"RAM::NES"
		// >> xFF
	};
}

#endif