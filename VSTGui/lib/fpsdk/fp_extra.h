#ifndef FP_EXTRA_H
#define FP_EXTRA_H



// to handle 32Bit int & float
union T32Bit 
{
    int i;
    float s;
	unsigned char buffer[4];
};



#define MaxPath 1024



// Ln(2) (for Power(2,n)) /12 (semitones per octave) /100 (cents)
//const double NoteMul = log(2)/1200.;
#define NoteMul 0.00057762267534



// mixer sample rate
#define MixSmpRate_Default 44100

// freq for middle C5
#define MiddleCFreq 523.251130601197
const double MiddleCMul = (float)0x10000000*MiddleCFreq*(float)0x10;

const double Div2 = 0.5;
const double Div4 = 0.25;
const double Div8 = 0.125;
const double Div10 = 0.1;
const double Div100 = 0.01;
const double Div128 = 1./128.;
const double Div256 = 1./256.;
const double Div1024 = 1./1024.;



int MinOf(int a, int b);

// translate FL voice volume to linear velocity (0..1)
float VolumeToVelocity(float Volume);
// translate FL voice volume to linear velocity (0..127)
float VolumeToMIDIVelocity(float Volume);



#endif  // FP_EXTRA_H
