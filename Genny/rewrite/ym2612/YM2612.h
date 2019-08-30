//#pragma once
//
//
//
//	//YM_DT = 0,
//	//YM_MUL = 1,
//	//YM_TL = 2,
//	//YM_DRUMTL = 3,
//	//YM_KS = 4,
//	//YM_AR = 5,
//	//YM_DR = 6,
//	//YM_SR = 7,
//	//YM_AM = 8,
//	//YM_SL = 9,
//	//YM_RR = 10,
//	//YM_F1 = 11,
//	//YM_F2 = 12,
//	//YM_SSG = 13,
//	//YM_AMS = 14,
//	//YM_FMS = 15,
//	//YM_L_EN = 16,
//	//YM_R_EN = 17,
//	//YM_FRQ = 18,
//	//YM_FB = 19,
//
//	//SN_SR = 20,
//	//SN_PERIODIC = 21,
//	//SN_DT = 22,
//
//	//YM_ALG = 23,
//	//YM_LFO_EN = 24,
//	//YM_LFO = 25, 
//	//YM_SPECIAL = 26,
//	//YM_FNUM1 = 27,
//	//YM_FNUM2 = 28,
//	//YM_NOTEON = 29,
//	//YM_DACEN = 30,
//	//YM_DAC = 31,
//	//YM_NONE
//
//
//enum EParam
//{
//
//};
//
//struct YMParam
//{
//	typedef enum {totalLevel, attackRate, decayRate, sustainLevel, secondaryDecayRate, releaseRate, detune, multiple, keyScalingRate, amplitudeModulationEnable, ssgEG } Type; 
//	YMParam(char pRange, YMParam::Type pType)
//	{
//		value = 0;
//		range = pRange;
//		type = pType;
//	};
//
//	char value;
//	char range;
//	YMParam::Type type;
//};
//
//struct YM2612Channel
//{
//
//};
//
//struct YM2612Operator
//{
//	YM2612Operator():
//		totalLevel(127, YMParam::totalLevel),
//		attackRate(31, YMParam::attackRate),
//		decayRate(31, YMParam::decayRate),
//		sustainLevel(15, YMParam::sustainLevel),
//		secondaryDecayRate(31, YMParam::secondaryDecayRate),
//		releaseRate(15, YMParam::releaseRate),	
//		detune(7, YMParam::detune),
//		multiple(15, YMParam::multiple),
//		keyScalingRate(3, YMParam::keyScalingRate),
//		amplitudeModulationEnable(1, YMParam::amplitudeModulationEnable),	
//		ssgEG(8, YMParam::ssgEG)
//	{
//	};
//
//	YMParam totalLevel; //(TL)
//	YMParam attackRate; //(AR)
//	YMParam decayRate; //(DR)	
//	YMParam sustainLevel; //(SL)
//	YMParam secondaryDecayRate; //(SR)
//	YMParam releaseRate; //(RR)
//	YMParam detune; //(DT)
//	YMParam multiple; //(MUL, Frequency)
//	YMParam keyScalingRate; //(KSR-RS-KS, ENV Scale) 
//	YMParam amplitudeModulationEnable; //(AM, LFO)
//	YMParam ssgEG; //(SSG)
//};
//
//class YM2612
//{
//public:
//	YM2612(void);
//	~YM2612(void);
//	virtual void initialize();
//};