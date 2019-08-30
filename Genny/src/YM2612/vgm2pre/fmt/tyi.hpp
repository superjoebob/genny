#ifndef __TYI_H__
#define __TYI_H__

#include <nall/nall.hpp>
using namespace nall;

/*******
TµEE co.(TM) YM2612 instrument file format specification.

Each file is 32 bytes long, containing data for straightforward loading
into YM2612 registers - very little code reqired to load an instrument.

Each entry except 8 and 9 are for all four operators.

1.  DT/MUL  - 4 - Detune(D6...D4) and multiplicator(D3...D0) value
2.  TL      - 4 - Total level (D6...D0)
3.  RS/AR   - 4 - Rate scaling(D7, D6) and Attack rate (D4...D0)
4.  AM/DR   - 4 - AM enable(D7) and Decay rate (D4...D0)
5.  SR      - 4 - Sustain rate (D4...D0)
6.  SL/RR   - 4 - Sustain level (D7...D4) and Release rate (D3...D0)
7.  SSG-EG  - 4 - SSG-EG value (D3...D0)
8.  FB/ALGO - 1 - Feedback (D5...D3) and algorithm (D2...D0)
9.  FMS/AMS - 1 - LFO AM (D5...D4) and FM (D2...D0)sensitivity
10. STRING  - 2 - "YI", ignored, for padding and optimization ;)

File extension should be TYI (Tiido's YM2612 Instrument file)
*******/

struct tyi_t {
	unsigned char dtmul[4];
	unsigned char tl[4];
	unsigned char rsar[4];
	unsigned char amdr[4];
	unsigned char sr[4];
	unsigned char slrr[4];
	unsigned char ssg[4];
	unsigned char fbalgo;
	unsigned char ms;
	unsigned char sig[2];
};

#endif