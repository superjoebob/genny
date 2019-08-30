/////* 
////    SN76489 emulation
////    by Maxim in 2001 and 2002
////*/
////
////#ifndef _SN76489_H_
////#define _SN76489_H_
////
////#define uint8  unsigned char
////#define uint16 unsigned short
////#define uint32 unsigned int
////#define int8  signed char
////#define int16 signed short
////#define int32 signed long int
////
////#define UINT8  unsigned char
////#define UINT16 unsigned short
////#define UINT32 unsigned int
////#define INT8  signed char
////#define INT16 signed short
////#define INT32 signed long int
////
////#define READ_BYTE(BASE, ADDR) (BASE)[ADDR]
////#define READ_WORD(BASE, ADDR) *(uint16 *)((BASE) + (ADDR))
////#define READ_WORD_LONG(BASE, ADDR) *(uint32 *)((BASE) + (ADDR))
////#define WRITE_BYTE(BASE, ADDR, VAL) (BASE)[ADDR] = VAL & 0xff
////#define WRITE_WORD(BASE, ADDR, VAL) *(uint16 *)((BASE) + (ADDR)) = VAL & 0xffff
////#define WRITE_WORD_LONG(BASE, ADDR, VAL) *(uint32 *)((BASE) + (ADDR)) = VAL & 0xffffffff
////
////#define M_PI 3.14159265358979323846264338327f
////
////#include <stdio.h>
////#include <stdlib.h>
////#include <string.h>
////#include <stdarg.h>
////#include <math.h>
////struct cfg
////{
////	cfg()
////		:psgBoostNoise(0)
////	{
////	}
////	int psgBoostNoise;
////};
////static cfg config;
////
////
/////* Fast sound synthesis buffer for use in real-time emulators of electronic
////sound generator chips like those in early video game consoles. Uses linear
////interpolation. Higher-quality versions are available that use sinc-based
////band-limited synthesis. */
////
////#ifndef BLIP_H
////#define BLIP_H
////
////#ifdef __cplusplus
////  extern "C" {
////#endif
////
/////* Creates a new blip_buffer with specified input clock rate, output
////sample rate, and size (in samples), or returns NULL if out of memory. */
////typedef struct blip_buffer_t blip_buffer_t;
////blip_buffer_t* blip_alloc( double clock_rate, double sample_rate, int size );
////
/////* Frees memory used by a blip_buffer. No effect if NULL is passed. */
////void blip_free( blip_buffer_t* );
////
/////* Removes all samples and clears buffer. */
////void blip_clear( blip_buffer_t* );
////
/////* Adds an amplitude transition of delta at specified time in source clocks.
////Delta can be negative. */
////void blip_add( blip_buffer_t*, int time, int delta );
////
/////* Number of additional clocks needed until n samples will be available.
////If buffer cannot even hold n samples, returns number of clocks until buffer
////becomes full. */
////int blip_clocks_needed( const blip_buffer_t*, int samples_needed );
////
/////* Ends current time frame of specified duration and make its samples available
////(along with any still-unread samples) for reading with read_samples(), then
////begins a new time frame at the end of the current frame. */
////void blip_end_frame( blip_buffer_t*, int duration );
////
/////* Number of samples available for reading with read(). */
////int blip_samples_avail( const blip_buffer_t* );
////
/////* Reads at most n samples out of buffer into out, removing them from from
////the buffer. */
////int blip_read_samples( blip_buffer_t*, short out [], int stereo);
////
////#ifdef __cplusplus
////  }
////#endif
////
////#endif
////
////
/////* Function prototypes */
////  static const uint16 PSGVolumeValues[16] =
////{
////  /* These values are taken from a real SMS2's output */
////  /*{892,892,892,760,623,497,404,323,257,198,159,123,96,75,60,0}, */
////  /* I can't remember why 892... :P some scaling I did at some point */
////  /* these values are true volumes for 2dB drops at each step (multiply previous by 10^-0.1) */
////  1516,1205,957,760,603,479,381,303,240,191,152,120,96,76,60,0
////};
////
////class I76489Impl
////{
////public:
/////* Initial state of shift register */
////#define NoiseInitialState 0x8000
////
/////* Value below which PSG does not output  */
/////*#define PSG_CUTOFF 0x6*/
////#define PSG_CUTOFF 0x1
////
/////* SN76489 clone in Sega's VDP chips (315-5124, 315-5246, 315-5313, Game Gear) */
////#define FB_SEGAVDP  0x0009
////#define SRW_SEGAVDP 16
////
////typedef struct
////{
////  /* Configuration */
////  int BoostNoise;         /* double noise volume when non-zero */
////
////  /* PSG registers: */
////  int Registers[8];       /* Tone, vol x4 */
////  int LatchedRegister;
////  int NoiseShiftRegister;
////  int NoiseFreq;          /* Noise channel signal generator frequency */
////
////  /* Output calculation variables */
////  int ToneFreqVals[4];    /* Frequency register values (counters) */
////  int ToneFreqPos[4];     /* Frequency channel flip-flops */
////  int Channels[4];        /* Value of each channel, before stereo is applied */
////
////  /* Blip-Buffer variables */
////  int chan_amp[4];        /* current channel amplitudes in delta buffers */
////} SN76489_Context;
////
////struct blip_buffer_t* blip;  /* delta resampler */
////
////SN76489_Context SN76489;
////
////void SN76489_Init(double PSGClockValue, int SamplingRate)
////{
////  SN76489_Shutdown();
////  
////  /* SamplingRate*16 instead of PSGClockValue/16 since division would lose some
////      precision. blip_alloc doesn't care about the absolute sampling rate, just the
////      ratio to clock rate. */
////  blip = blip_alloc(PSGClockValue, SamplingRate * 16.0, SamplingRate / 4);
////}
////
////void SN76489_Shutdown(void)
////{
////  if (blip) blip_free(blip);
////  blip = NULL;
////}
////
////void SN76489_Reset()
////{
////  int i;
////
////  for(i = 0; i <= 3; i++)
////  {
////    /* Initialise PSG state */
////    SN76489.Registers[2*i] = 1;         /* tone freq=1 */
////    SN76489.Registers[2*i+1] = 0xf;     /* vol=off */
////
////   /* Set counters to 0 */
////    SN76489.ToneFreqVals[i] = 0;
////
////   /* Set flip-flops to 1 */
////    SN76489.ToneFreqPos[i] = 1;
////
////   /* Clear channels output */
////    SN76489.Channels[i] = 0;
////
////   /* Clear current amplitudes in delta buffer */
////    SN76489.chan_amp[i] = 0;
////  }
////
////  SN76489.LatchedRegister=0;
////
////  /* Initialise noise generator */
////  SN76489.NoiseShiftRegister=NoiseInitialState;
////  SN76489.NoiseFreq = 0x10;
////  SN76489.BoostNoise = config.psgBoostNoise;
////
////  /* Clear Blip delta buffer */
////  if (blip) blip_clear(blip);
////}
////
////void SN76489_BoostNoise(int boost)
////{
////  SN76489.BoostNoise = boost;
////  SN76489.Channels[3]= PSGVolumeValues[SN76489.Registers[7]] << boost;
////}
////
////void SN76489_SetContext(uint8 *data)
////{
////  memcpy(&SN76489, data, sizeof(SN76489_Context));
////}
////
////void SN76489_GetContext(uint8 *data)
////{
////  memcpy(data, &SN76489, sizeof(SN76489_Context));
////}
////
////uint8 *SN76489_GetContextPtr(void)
////{
////  return (uint8 *)&SN76489;
////}
////
////int SN76489_GetContextSize(void)
////{
////  return sizeof(SN76489_Context);
////}
////
////void SN76489_Write(int data)
////{
////  if (data & 0x80)
////  {
////    /* Latch byte  %1 cc t dddd */
////    SN76489.LatchedRegister = (data >> 4) & 0x07;
////  }
////
////  switch (SN76489.LatchedRegister)
////  {
////    case 0:
////    case 2:
////    case 4: /* Tone channels */
////      if (data & 0x80)
////      {
////        /* Data byte  %1 cc t dddd */
////        SN76489.Registers[SN76489.LatchedRegister] = (SN76489.Registers[SN76489.LatchedRegister] & 0x3f0) | (data & 0xf);
////      }
////      else
////      {
////        /* Data byte  %0 - dddddd */
////        SN76489.Registers[SN76489.LatchedRegister] = (SN76489.Registers[SN76489.LatchedRegister] & 0x00f) | ((data & 0x3f) << 4);
////      }
////      /* Zero frequency changed to 1 to avoid div/0 */
////      if (SN76489.Registers[SN76489.LatchedRegister] == 0) SN76489.Registers[SN76489.LatchedRegister] = 1;  
////      break;
////
////    case 1:
////    case 3:
////    case 5: /* Channel attenuation */
////      SN76489.Registers[SN76489.LatchedRegister] = data & 0x0f;
////      SN76489.Channels[SN76489.LatchedRegister>>1] = PSGVolumeValues[data&0x0f];
////      break;
////
////    case 6: /* Noise control */
////      SN76489.Registers[6] = data & 0x0f;
////      SN76489.NoiseShiftRegister = NoiseInitialState;  /* reset shift register */
////      SN76489.NoiseFreq = 0x10 << (data&0x3); /* set noise signal generator frequency */
////      break;
////
////    case 7: /* Noise attenuation */
////      SN76489.Registers[7] = data & 0x0f;
////      SN76489.Channels[3] = PSGVolumeValues[data&0x0f] << SN76489.BoostNoise;
////      break;
////  }
////}
////
/////* Updates tone amplitude in delta buffer. Call whenever amplitude might have changed. */
////void UpdateToneAmplitude(int i, int time)
////{
////  int delta = (SN76489.Channels[i] * SN76489.ToneFreqPos[i]) - SN76489.chan_amp[i];
////  if (delta != 0)
////  {
////    SN76489.chan_amp[i] += delta;
////    blip_add(blip, time, delta);
////  }
////}
////
/////* Updates noise amplitude in delta buffer. Call whenever amplitude might have changed. */
////void UpdateNoiseAmplitude(int time)
////{
////  int delta = (SN76489.Channels[3] * ( SN76489.NoiseShiftRegister & 0x1 )) - SN76489.chan_amp[3];
////  if (delta != 0)
////  {
////    SN76489.chan_amp[3] += delta;
////    blip_add(blip, time, delta);
////  }
////}
////
/////* Runs tone channel for clock_length clocks */
////void RunTone(int i, int clock_length)
////{
////  int time;
////
////  /* Update in case a register changed etc. */
////  UpdateToneAmplitude(i, 0);
////
////  /* Time of next transition */
////  time = SN76489.ToneFreqVals[i];
////
////  /* Process any transitions that occur within clocks we're running */
////  while (time < clock_length)
////  {
////    if (SN76489.Registers[i*2]>PSG_CUTOFF) {
////      /* Flip the flip-flop */
////      SN76489.ToneFreqPos[i] = -SN76489.ToneFreqPos[i];
////    } else {
////      /* stuck value */
////      SN76489.ToneFreqPos[i] = 1;
////    }
////    UpdateToneAmplitude(i, time);
////
////    /* Advance to time of next transition */
////    time += SN76489.Registers[i*2];
////  }
////  
////  /* Calculate new value for register, now that next transition is past number of clocks we're running */
////  SN76489.ToneFreqVals[i] = time - clock_length;
////}
////
/////* Runs noise channel for clock_length clocks */
////void RunNoise(int clock_length)
////{
////  int time;
////
////  /* Noise channel: match to tone2 if in slave mode */
////  int NoiseFreq = SN76489.NoiseFreq;
////  if (NoiseFreq == 0x80)
////  {
////    NoiseFreq = SN76489.Registers[2*2];
////    SN76489.ToneFreqVals[3] = SN76489.ToneFreqVals[2];
////  }
////
////  /* Update in case a register changed etc. */
////  UpdateNoiseAmplitude(0);
////
////  /* Time of next transition */
////  time = SN76489.ToneFreqVals[3];
////
////  /* Process any transitions that occur within clocks we're running */
////  while ( time < clock_length )
////  {
////    /* Flip the flip-flop */
////    SN76489.ToneFreqPos[3] = -SN76489.ToneFreqPos[3];
////    if (SN76489.ToneFreqPos[3] == 1)
////    {
////      /* On the positive edge of the square wave (only once per cycle) */
////      int Feedback = SN76489.NoiseShiftRegister;
////      if ( SN76489.Registers[6] & 0x4 )
////      {
////        /* White noise */
////        /* Calculate parity of fed-back bits for feedback */
////        /* Do some optimised calculations for common (known) feedback values */
////        /* If two bits fed back, I can do Feedback=(nsr & fb) && (nsr & fb ^ fb) */
////        /* since that's (one or more bits set) && (not all bits set) */
////        Feedback = ((Feedback & FB_SEGAVDP) && ((Feedback & FB_SEGAVDP) ^ FB_SEGAVDP));
////      }
////      else    /* Periodic noise */
////        Feedback = Feedback & 1;
////
////      SN76489.NoiseShiftRegister = (SN76489.NoiseShiftRegister >> 1) | (Feedback << (SRW_SEGAVDP - 1));
////      UpdateNoiseAmplitude(time);
////    }
////
////    /* Advance to time of next transition */
////    time += NoiseFreq;
////  }
////
////  /* Calculate new value for register, now that next transition is past number of clocks we're running */
////  SN76489.ToneFreqVals[3] = time - clock_length;
////}
////
////int SN76489_Update(INT16 *buffer, int clock_length)
////{
////  int i;
////
////  /* Run noise first, since it might use current value of third tone frequency counter */
////  RunNoise(clock_length);
////
////  /* Run tone channels */
////  for( i = 0; i <= 2; ++i )
////    RunTone(i, clock_length);
////
////  /* Read samples into output buffer */
////  blip_end_frame(blip, clock_length);
////  return blip_read_samples(blip, buffer, 0);
////}
////
////int SN76489_Clocks(int length)
////{
////  return blip_clocks_needed(blip, length);
////}
////
////
////};
////
////#endif /* _SN76489_H_ */




#ifndef _SN76489_H_
#define _SN76489_H_
#include <stdint.h>
#define MAX_SN76489     4

/* These values are taken from a real SMS2's output */
static const int PSGVolumeValues[2][16] = {
    {892,892,892,760,623,497,404,323,257,198,159,123,96,75,60,0}, /* I can't remember why 892... :P some scaling I did at some point */
	{892,774,669,575,492,417,351,292,239,192,150,113,80,50,24,0}
};

class I76489Impl
{
/*
    More testing is needed to find and confirm feedback patterns for
    SN76489 variants and compatible chips.
*/
enum feedback_patterns {
    FB_BBCMICRO =   0x8005, /* Texas Instruments TMS SN76489N (original) from BBC Micro computer */
    FB_SC3000   =   0x0006, /* Texas Instruments TMS SN76489AN (rev. A) from SC-3000H computer */
    FB_SEGAVDP  =   0x0009, /* SN76489 clone in Sega's VDP chips (315-5124, 315-5246, 315-5313, Game Gear) */
};

enum volume_modes {
    VOL_TRUNC   =   0,      /* Volume levels 13-15 are identical */
    VOL_FULL    =   1,      /* Volume levels 13-15 are unique */
};

enum boost_modes {
    BOOST_OFF   =   0,      /* Regular noise channel volume */
    BOOST_ON    =   1,      /* Doubled noise channel volume */
};

enum mute_values {
    MUTE_ALLOFF =   0,      /* All channels muted */
    MUTE_TONE1  =   1,      /* Tone 1 mute control */
    MUTE_TONE2  =   2,      /* Tone 2 mute control */
    MUTE_TONE3  =   4,      /* Tone 3 mute control */
    MUTE_NOISE  =   8,      /* Noise mute control */
    MUTE_ALLON  =   15,     /* All channels enabled */
};

//typedef struct
//{
//    /* expose this for inspection/modification for channel muting */
//
//
//} SN76489_Context;

    int Mute;
    int BoostNoise;
    int VolumeArray;
    
    /* Variables */
    float Clock;
    float dClock;
    int PSGStereo;
    int NumClocksForSample;
    int WhiteNoiseFeedback;
    
    /* PSG registers: */
    uint16_t Registers[8];        /* Tone, vol x4 */
    int LatchedRegister;
    uint16_t NoiseShiftRegister;
    int16_t NoiseFreq;            /* Noise channel signal generator frequency */
    
    /* Output calculation variables */
    int16_t ToneFreqVals[4];      /* Frequency register values (counters) */
    int8_t ToneFreqPos[4];        /* Frequency channel flip-flops */
    int16_t Channels[4];          /* Value of each channel, before stereo is applied */
    int32_t IntermediatePos[4];   /* intermediate values used at boundaries between + and - */

#define NoiseInitialState   0x8000  /* Initial state of shift register */
#define PSG_CUTOFF          0x6     /* Value below which PSG does not output */



//static SN76489_Context SN76489[MAX_SN76489];

public:

/* Function prototypes */
void SN76489_Init(int PSGClockValue, int SamplingRate)
{
    dClock=(float)PSGClockValue/16/SamplingRate;
    SN76489_Config(MUTE_ALLON, BOOST_ON, VOL_FULL, FB_SEGAVDP);
    SN76489_Reset();

	sn_enablePerNotePanning = true;
	sn_perNoteVolumeL[0] = 0;
	sn_perNoteVolumeL[1] = 0;
	sn_perNoteVolumeL[2] = 0;
	sn_perNoteVolumeL[3] = 0;
	sn_perNoteVolumeR[0] = 0;
	sn_perNoteVolumeR[1] = 0;
	sn_perNoteVolumeR[2] = 0;
	sn_perNoteVolumeR[3] = 0;
}
void SN76489_Reset()
{
    int i;

    PSGStereo = 0xFF;

    for(i = 0; i <= 3; i++)
    {
        /* Initialise PSG state */
        Registers[2*i] = 1;         /* tone freq=1 */
        Registers[2*i+1] = 0xf;     /* vol=off */
        NoiseFreq = 0x10;

        /* Set counters to 0 */
        ToneFreqVals[i] = 0;

        /* Set flip-flops to 1 */
        ToneFreqPos[i] = 1;

        /* Set intermediate positions to do-not-use value */
        IntermediatePos[i] = -1;
    }

    LatchedRegister=0;

    /* Initialise noise generator */
    NoiseShiftRegister=NoiseInitialState;

    /* Zero clock */
    Clock=0;

}
void SN76489_Shutdown(void)
{
}
void SN76489_Config(int mute, int boost, int volume, int feedback)
{
    Mute = mute;
    BoostNoise = boost;
    VolumeArray = volume;
    WhiteNoiseFeedback = feedback;
}

void SN76489_Write(int data)
{
	if (data&0x80) {
        /* Latch/data byte  %1 cc t dddd */
        LatchedRegister=((data>>4)&0x07);
        Registers[LatchedRegister]=
            (Registers[LatchedRegister] & 0x3f0)    /* zero low 4 bits */
            | (data&0xf);                           /* and replace with data */
	} else {
        /* Data byte        %0 - dddddd */
        if (!(LatchedRegister%2)&&(LatchedRegister<5))
            /* Tone register */
            Registers[LatchedRegister]=
                (Registers[LatchedRegister] & 0x00f)    /* zero high 6 bits */
                | ((data&0x3f)<<4);                     /* and replace with data */
		else
            /* Other register */
            Registers[LatchedRegister]=data&0x0f;       /* Replace with data */
    }
    switch (LatchedRegister) {
	case 0:
	case 2:
    case 4: /* Tone channels */
        if (Registers[LatchedRegister]==0) Registers[LatchedRegister]=1;    /* Zero frequency changed to 1 to avoid div/0 */
		break;
    case 6: /* Noise */
        NoiseShiftRegister=NoiseInitialState;   /* reset shift register */
        NoiseFreq=0x10<<(Registers[6]&0x3);     /* set noise signal generator frequency */
		break;
    }
}
void SN76489_GGStereoWrite(int data)
{
    PSGStereo=data;
}

float  sn_perNoteVolumeL[4];  /* superjoebob adds per note panning */
float  sn_perNoteVolumeR[4];
bool sn_enablePerNotePanning;

void SN76489_Update(int16_t **buffer, int length)
{
    int i, j;

    for(j = 0; j < length; j++)
    {
        for (i=0;i<=2;++i)
            if (IntermediatePos[i] != -1)
                Channels[i]=(Mute >> i & 0x1)*PSGVolumeValues[VolumeArray][Registers[2*i+1]]*IntermediatePos[i]/65536;
            else
                Channels[i]=(Mute >> i & 0x1)*PSGVolumeValues[VolumeArray][Registers[2*i+1]]*ToneFreqPos[i];
    
        Channels[3]=(short)((Mute >> 3 & 0x1)*PSGVolumeValues[VolumeArray][Registers[7]]*(NoiseShiftRegister & 0x1));
    
        if (BoostNoise) Channels[3]<<=1; /* Double noise volume to make some people happy */
    
        buffer[0][j] =0;
        buffer[1][j] =0;

		if(sn_enablePerNotePanning)
		{
			for (i=0;i<=3;++i) {
				buffer[0][j] +=((PSGStereo >> (i+4) & 0x1)*Channels[i]) * sn_perNoteVolumeL[i]; /* left */
				buffer[1][j] +=((PSGStereo >>  i    & 0x1)*Channels[i]) * sn_perNoteVolumeR[i]; /* right */
			}
		}
		else
		{
			for (i=0;i<=3;++i) {
				buffer[0][j] +=((PSGStereo >> (i+4) & 0x1)*Channels[i]); /* left */
				buffer[1][j] +=((PSGStereo >>  i    & 0x1)*Channels[i]); /* right */
			}
		}
        Clock+=dClock; 
        NumClocksForSample=(int)Clock;  /* truncates */
        Clock-=NumClocksForSample;  /* remove integer part */
        /* Looks nicer in Delphi... */
        /*  Clock:=Clock+dClock; */
        /*  NumClocksForSample:=Trunc(Clock); */
        /*  Clock:=Frac(Clock); */
    
        /* Decrement tone channel counters */
        for (i=0;i<=2;++i)
            ToneFreqVals[i]-=NumClocksForSample;
     
        /* Noise channel: match to tone2 or decrement its counter */
        if (NoiseFreq==0x80) ToneFreqVals[3]=ToneFreqVals[2];
        else ToneFreqVals[3]-=NumClocksForSample;
    
        /* Tone channels: */
        for (i=0;i<=2;++i) {
            if (ToneFreqVals[i]<=0) {   /* If it gets below 0... */
                if (Registers[i*2]>PSG_CUTOFF) {
                    /* Calculate how much of the sample is + and how much is - */
                    /* Go to floating point and include the clock fraction for extreme accuracy :D */
                    /* Store as long int, maybe it's faster? I'm not very good at this */
                    IntermediatePos[i]=(long)((NumClocksForSample-Clock+2*ToneFreqVals[i])*ToneFreqPos[i]/(NumClocksForSample+Clock)*65536);
                    ToneFreqPos[i]=-ToneFreqPos[i]; /* Flip the flip-flop */
                } else {
                    ToneFreqPos[i]=1;   /* stuck value */
                    IntermediatePos[i] = -1;
                }
                ToneFreqVals[i]+=Registers[i*2]*(NumClocksForSample/Registers[i*2]+1);
            } else IntermediatePos[i]= -1;
        }
    
        /* Noise channel */
        if (ToneFreqVals[3]<=0) {   /* If it gets below 0... */
            ToneFreqPos[3]=-ToneFreqPos[3]; /* Flip the flip-flop */
            if (NoiseFreq!=0x80)            /* If not matching tone2, decrement counter */
                ToneFreqVals[3]+=NoiseFreq*(NumClocksForSample/NoiseFreq+1);
            if (ToneFreqPos[3]==1) {    /* Only once per cycle... */
                int Feedback;
                if (Registers[6]&0x4) { /* White noise */
                    /* Calculate parity of fed-back bits for feedback */
                    switch (WhiteNoiseFeedback) {
                        /* Do some optimised calculations for common (known) feedback values */
                    case 0x0006:    /* SC-3000      %00000110 */
                    case 0x0009:    /* SMS, GG, MD  %00001001 */
                        /* If two bits fed back, I can do Feedback=(nsr & fb) && (nsr & fb ^ fb) */
                        /* since that's (one or more bits set) && (not all bits set) */
    /* which one?         Feedback=((NoiseShiftRegister&WhiteNoiseFeedback) && (NoiseShiftRegister&WhiteNoiseFeedback^WhiteNoiseFeedback)); */
                        Feedback=((NoiseShiftRegister&WhiteNoiseFeedback) && ((NoiseShiftRegister&WhiteNoiseFeedback)^WhiteNoiseFeedback));
                        break;
                    case 0x8005:    /* BBC Micro */
                        /* fall through :P can't be bothered to think too much */
                    default:        /* Default handler for all other feedback values */
                        Feedback=NoiseShiftRegister&WhiteNoiseFeedback;
                        Feedback^=Feedback>>8;
                        Feedback^=Feedback>>4;
                        Feedback^=Feedback>>2;
                        Feedback^=Feedback>>1;
                        Feedback&=1;
                        break;
                    }
                } else      /* Periodic noise */
                    Feedback=NoiseShiftRegister&1;
    
                NoiseShiftRegister=(NoiseShiftRegister>>1) | (Feedback<<15);
    
    /* Original code: */
    /*          NoiseShiftRegister=(NoiseShiftRegister>>1) | ((Registers[6]&0x4?((NoiseShiftRegister&0x9) && (NoiseShiftRegister&0x9^0x9)):NoiseShiftRegister&1)<<15); */
            }
        }
    }
}
};

#endif /* _SN76489_H_ */