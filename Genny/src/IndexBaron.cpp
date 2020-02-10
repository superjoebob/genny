#include "IndexBaron.h"
#include "Genny2612.h"

std::string IBYMParam::getValue(GennyPatch* patch)
{
	std::string val;
	float fval = patch->getFromBaron(this);
	int ival = (int)(fval + 0.5f);
	char buf[6];
	itoa(ival, buf, 10);
	return buf;
}

IndexBaron::IndexBaron(void)
	:_currentInstrument(-1)
	,enableTrueStereo(false)
{
}


IndexBaron::~IndexBaron(void)
{
	for(size_t i = 0; i < _catalogue.size(); i++)
	{
		delete _catalogue[i];
	}
}

int IndexBaron::getYMParamIndex(YM2612Param param, int op)
{
	for(size_t i = 0; i < _catalogue.size(); i++)
	{
		if(_catalogue[i]->getType() == IB_YMParam)
		{
			IBYMParam* p = static_cast<IBYMParam*>(_catalogue[i]);
			if(p->getParameter() == param && p->getOperator() == op)
				return i;
		}
	}
	return -1;
}

int IndexBaron::getInsParamIndex(GennyInstrumentParam param)
{
	for(size_t i = 0; i < _catalogue.size(); i++)
	{
		if(_catalogue[i]->getType() == IB_InsParam)
		{
			IBInsParam* p = static_cast<IBInsParam*>(_catalogue[i]);
			if(p->getParameter() == param)
				return i;
		}
	}
	return -1;
}

int IndexBaron::getPatchParamIndex(GennyPatchParam param)
{
	for(size_t i = 0; i < _catalogue.size(); i++)
	{
		if(_catalogue[i]->getType() == IB_PatchParam)
		{
			IBPatchParam* p = static_cast<IBPatchParam*>(_catalogue[i]);
			if(p->getParameter() == param)
				return i;
		}
	}
	return -1;
}

IBIndex* IndexBaron::getIndexFromMidi(int midi)
{
	switch(midi)
	{
	case 74: return getIndex(getYMParamIndex(YM_LFO_EN));
	case 1: return getIndex(getYMParamIndex(YM_LFO));
	case 80: return getIndex(getYMParamIndex(YM_SPECIAL));
	case 14: return getIndex(getYMParamIndex(YM_ALG));
	case 15: return getIndex(getYMParamIndex(YM_FB));
	case 76: return getIndex(getYMParamIndex(YM_AMS));
	case 75: return getIndex(getYMParamIndex(YM_FMS));


	case 16: return getIndex(getYMParamIndex(YM_TL, 0));
	case 17: return getIndex(getYMParamIndex(YM_TL, 1));
	case 18: return getIndex(getYMParamIndex(YM_TL, 2));
	case 19: return getIndex(getYMParamIndex(YM_TL, 3));

	case 20: return getIndex(getYMParamIndex(YM_MUL, 0));
	case 21: return getIndex(getYMParamIndex(YM_MUL, 1));
	case 22: return getIndex(getYMParamIndex(YM_MUL, 2));
	case 23: return getIndex(getYMParamIndex(YM_MUL, 3));

	case 24: return getIndex(getYMParamIndex(YM_DT, 0));
	case 25: return getIndex(getYMParamIndex(YM_DT, 1));
	case 26: return getIndex(getYMParamIndex(YM_DT, 2));
	case 27: return getIndex(getYMParamIndex(YM_DT, 3));

	case 39: return getIndex(getYMParamIndex(YM_KS, 0));
	case 40: return getIndex(getYMParamIndex(YM_KS, 1));
	case 41: return getIndex(getYMParamIndex(YM_KS, 2));
	case 42: return getIndex(getYMParamIndex(YM_KS, 3));

	case 43: return getIndex(getYMParamIndex(YM_AR, 0));
	case 44: return getIndex(getYMParamIndex(YM_AR, 1));
	case 45: return getIndex(getYMParamIndex(YM_AR, 2));
	case 46: return getIndex(getYMParamIndex(YM_AR, 3));

	case 47: return getIndex(getYMParamIndex(YM_DR, 0));
	case 48: return getIndex(getYMParamIndex(YM_DR, 1));
	case 49: return getIndex(getYMParamIndex(YM_DR, 2));
	case 50: return getIndex(getYMParamIndex(YM_DR, 3));

	case 51: return getIndex(getYMParamIndex(YM_SR, 0));
	case 52: return getIndex(getYMParamIndex(YM_SR, 1));
	case 53: return getIndex(getYMParamIndex(YM_SR, 2));
	case 54: return getIndex(getYMParamIndex(YM_SR, 3));

	case 55: return getIndex(getYMParamIndex(YM_SL, 0));
	case 56: return getIndex(getYMParamIndex(YM_SL, 1));
	case 57: return getIndex(getYMParamIndex(YM_SL, 2));
	case 58: return getIndex(getYMParamIndex(YM_SL, 3));

	case 59: return getIndex(getYMParamIndex(YM_RR, 0));
	case 60: return getIndex(getYMParamIndex(YM_RR, 1));
	case 61: return getIndex(getYMParamIndex(YM_RR, 2));
	case 62: return getIndex(getYMParamIndex(YM_RR, 3));

	case 70: return getIndex(getYMParamIndex(YM_AM, 0));
	case 71: return getIndex(getYMParamIndex(YM_AM, 1));
	case 72: return getIndex(getYMParamIndex(YM_AM, 2));
	case 73: return getIndex(getYMParamIndex(YM_AM, 3));

	case 81: return getIndex(getYMParamIndex(YM_FRQ));
	}
	return nullptr;
}



char IndexBaron::getCCfromParam(IBIndex* param)
{
	IBType t = param->getType();
	if(t == IBType::IB_YMParam)
	{
		IBYMParam* ym = (IBYMParam*)param;
		switch(ym->getParameter())
		{
			case YM2612Param::YM_DT: return 24 + ym->getOperator();
			case YM2612Param::YM_MUL: return 20 + ym->getOperator();
			case YM2612Param::YM_TL: return 16 + ym->getOperator();
			case YM2612Param::YM_KS : return 39 + ym->getOperator();
			case YM2612Param::YM_AR : return 43 + ym->getOperator();
			case YM2612Param::YM_DR : return 47 + ym->getOperator();
			case YM2612Param::YM_SR : return 51 + ym->getOperator();
			case YM2612Param::YM_AM : return 70 + ym->getOperator();
			case YM2612Param::YM_SL : return 55 + ym->getOperator();
			case YM2612Param::YM_RR : return 59 + ym->getOperator();
			case YM2612Param::YM_SSG : return 90 + ym->getOperator();
			case YM2612Param::YM_AMS : return 76;
			case YM2612Param::YM_FMS : return 75;

			//case YM2612Param::YM_L_EN : return 16 + ym->getOperator();
			//case YM2612Param::YM_R_EN : return 16 + ym->getOperator();
			//case YM2612Param::YM_FRQ : return 16 + ym->getOperator();
			case YM2612Param::YM_FB: return 15;


			case YM2612Param::YM_ALG : return 14;
			//case YM2612Param::YM_LFO_EN : return 74;
			//case YM2612Param::YM_LFO : return 1; 
			//case YM2612Param::YM_SPECIAL : return 80;
		}
	}	

	return 0;
}

char IndexBaron::getCCRange(IBIndex* param)
{
	IBType t = param->getType();
	if(t == IBType::IB_YMParam)
	{
		IBYMParam* ym = (IBYMParam*)param;
		switch(ym->getParameter())
		{
			case YM2612Param::YM_DT: return 7;
			case YM2612Param::YM_MUL: return 15;
			case YM2612Param::YM_TL: return 127;
			case YM2612Param::YM_KS : return 3;
			case YM2612Param::YM_AR : return 31;
			case YM2612Param::YM_DR : return 31;
			case YM2612Param::YM_SR : return 15;
			case YM2612Param::YM_AM : return 1;
			case YM2612Param::YM_SL : return 15;
			case YM2612Param::YM_RR : return 15;
			case YM2612Param::YM_SSG : return 15;
			case YM2612Param::YM_AMS : return 7;
			case YM2612Param::YM_FMS : return 7;

			//case YM2612Param::YM_L_EN : return 16 + ym->getOperator();
			//case YM2612Param::YM_R_EN : return 16 + ym->getOperator();
			//case YM2612Param::YM_FRQ : return 16 + ym->getOperator();
			case YM2612Param::YM_FB: return 7;


			case YM2612Param::YM_ALG : return 7;
			case YM2612Param::YM_LFO_EN : return 1;
			case YM2612Param::YM_LFO : return 7; 
			case YM2612Param::YM_SPECIAL : return 1;
		}
	}	
}


int IndexBaron::getIBIndex(IBIndex* index)
{
	for(int i = 0; i < _catalogue.size(); i++)
	{
		if(_catalogue[i] == index)
			return i;
	}
	return -1;
}