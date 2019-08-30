/* Finite impulse response (FIR) resampler with adjustable FIR size */

/* Game_Music_Emu 0.5.2. http://www.slack.net/~ant/ */

/* Copyright (C) 2004-2006 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details. You should have received a copy of the GNU Lesser General Public
License along with this module; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */

/* C Conversion by Eke-Eke for use in Genesis Plus GX (2009). */

#ifndef FIR_RESAMPLER_H
#define FIR_RESAMPLER_H

#ifdef LSB_FIRST

#define READ_BYTE(BASE, ADDR) (BASE)[(ADDR)^1]

#define READ_WORD(BASE, ADDR) (((BASE)[ADDR]<<8) | (BASE)[(ADDR)+1])

#define READ_WORD_LONG(BASE, ADDR) (((BASE)[(ADDR)+1]<<24) |      \
                                    ((BASE)[(ADDR)]<<16) |  \
                                    ((BASE)[(ADDR)+3]<<8) |   \
                                    (BASE)[(ADDR)+2])

#define WRITE_BYTE(BASE, ADDR, VAL) (BASE)[(ADDR)^1] = (VAL)&0xff

#define WRITE_WORD(BASE, ADDR, VAL) (BASE)[ADDR] = ((VAL)>>8) & 0xff; \
                                      (BASE)[(ADDR)+1] = (VAL)&0xff

#define WRITE_WORD_LONG(BASE, ADDR, VAL) (BASE)[(ADDR+1)] = ((VAL)>>24) & 0xff;    \
                                          (BASE)[(ADDR)] = ((VAL)>>16)&0xff;  \
                                          (BASE)[(ADDR+3)] = ((VAL)>>8)&0xff;   \
                                          (BASE)[(ADDR+2)] = (VAL)&0xff

#else

#define READ_BYTE(BASE, ADDR) (BASE)[ADDR]
#define READ_WORD(BASE, ADDR) *(uint16 *)((BASE) + (ADDR))
#define READ_WORD_LONG(BASE, ADDR) *(uint32 *)((BASE) + (ADDR))
#define WRITE_BYTE(BASE, ADDR, VAL) (BASE)[ADDR] = VAL & 0xff
#define WRITE_WORD(BASE, ADDR, VAL) *(uint16 *)((BASE) + (ADDR)) = VAL & 0xffff
#define WRITE_WORD_LONG(BASE, ADDR, VAL) *(uint32 *)((BASE) + (ADDR)) = VAL & 0xffffffff
#endif

/* C89 compatibility */
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327f
#endif /* M_PI */

/* Set to your compiler's static inline keyword to enable it, or
 * set it to blank to disable it.
 * If you define INLINE in the makefile, it will override this value.
 * NOTE: not enabling inline functions will SEVERELY slow down emulation.
 */
#ifndef INLINE
#define INLINE static __inline__
#endif /* INLINE */

#define STEREO        2
#define MAX_RES       32
#define WIDTH         16
#define WRITE_OFFSET  (WIDTH * STEREO) - STEREO
#define GAIN          1.0

typedef signed int sample_t;

class Resampler
{
public:
	Resampler()
		:buffer(nullptr)
		,buffer_size(0)
	    ,write_pos(nullptr)
		,res(1)
	    ,imp_phase(0)
	    ,skip_bits(0)
	    ,step(STEREO)
	    ,ratio(1.0)
	{

	}

	int Fir_Resampler_initialize( int new_size );
	void Fir_Resampler_shutdown( void );
	void Fir_Resampler_clear( void );
	double Fir_Resampler_time_ratio( double new_factor, double rolloff );
	double Fir_Resampler_ratio( void );
	int Fir_Resampler_max_write( void );
	sample_t* Fir_Resampler_buffer( void );
	int Fir_Resampler_written( void );
	int Fir_Resampler_avail( void );
	void Fir_Resampler_write( long count );
	int Fir_Resampler_read( sample_t* out, long count );
	int Fir_Resampler_input_needed( long output_count );
	int Fir_Resampler_skip_input( long count );

private:

	/* sound buffer */
	sample_t *buffer;
	int buffer_size;

	sample_t impulses[MAX_RES][WIDTH];
	sample_t* write_pos;
	int res;
	int imp_phase;
	unsigned long skip_bits;
	int step;
	int input_per_cycle;
	double ratio;
};

#endif
