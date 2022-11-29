#pragma once
#include "YM2612/YM2612.h"
#include "Genny2612.h"

enum IBType
{
	IB_PatchParam = 0,
	IB_YMParam = 1,
	IB_InsParam = 2
};



class IndexBaron;
class IBIndex
{
public:
	IBIndex(std::string name = "", int vMinValue = 0, int vMaxValue = 0) :_name(name), minValue(vMinValue), maxValue(vMaxValue), global(false) { }
	inline IBType getType() const { return _type; }
	std::string getName() const { return _name; }

	virtual ParamDisplayType getDisplayType() { return ParamDisplayType::Integer; }

	virtual std::string getValue(GennyPatch* patch) { return ""; }

	int minValue;
	int maxValue;
	bool global;

protected:
	IBType _type;
	std::string _name;
};


class IBPatchParam : public IBIndex
{
public:
	IBPatchParam(GennyPatchParam param, int vMinValue, int vMaxValue, std::string name = "")
		:IBIndex(name, vMinValue, vMaxValue)
		,_parameter(param)
	{ 
		_type = IB_PatchParam; 
	}
	GennyPatchParam getParameter() const { return _parameter; }

private:
	GennyPatchParam _parameter;
};

class IBInsParam : public IBIndex
{
public:
	IBInsParam(char instrument, int vMinValue, int vMaxValue, std::string name = "")
		:IBIndex(name, vMinValue, vMaxValue)
	    ,_instrument(instrument)
		,_parameter(GIP_None)
	{ 
		_type = IB_InsParam; 
	}

	IBInsParam(char instrument, GennyInstrumentParam param, int vMinValue, int vMaxValue, std::string name = "")
		:IBIndex(name, vMinValue, vMaxValue)
	    ,_instrument(instrument)
		,_parameter(param)
	{ 
		_type = IB_InsParam; 
	}

	char getInstrument() const { return _instrument; }
	GennyInstrumentParam getParameter() const { return _parameter; }

	virtual ParamDisplayType getDisplayType() 
	{ 
		switch (_parameter)
		{
		case GIP_Enabled:
		case GIP_DAC:				
		case GIP_Channel0:
		case GIP_Channel1:
		case GIP_Channel2:
		case GIP_Channel3:
		case GIP_Channel4:
		case GIP_Channel5:
		case GIP_Channel6:
		case GIP_Channel7:
		case GIP_Channel8:
		case GIP_Channel9:
		case GIP_FM:
			return ParamDisplayType::Bool;

		case GIP_Octave:
		case GIP_Ch3OpOctave:
			return (ParamDisplayType)((int)ParamDisplayType::Octave | (int)ParamDisplayType::Centered);
		case GIP_Transpose:
		case GIP_Ch3OpTranspose:
			return (ParamDisplayType)((int)ParamDisplayType::Semitone | (int)ParamDisplayType::Centered);
		case GIP_Panning:
			return ParamDisplayType::Panning;

		case GIP_RangeLow:
		case GIP_RangeHigh:
			return ParamDisplayType::MidiRange;
		}

		return ParamDisplayType::Integer; 
	}

protected:
	char _instrument;

private:
	GennyInstrumentParam _parameter;
};


class IBYMParam : public IBInsParam
{
public:
	IBYMParam(YM2612Param param, char op, char instrument, int vMinValue, int vMaxValue, std::string name = "")
	    :IBInsParam(instrument, vMinValue, vMaxValue, name)
		,_parameter(param)
		,_op(op)
	{ 
		_type = IB_YMParam; 
	}

	YM2612Param getParameter() const { return _parameter; }
	char getOperator() const { return _op; }
	virtual std::string getValue(GennyPatch* patch);

	virtual ParamDisplayType getDisplayType()
	{
		switch (_parameter)
		{
		case YM_LFO_EN:
		case YM_DACEN:
		case YM_DAC:
			return ParamDisplayType::Bool;

		case YM_DT:
			return ParamDisplayType::DT_FUN;

		case YM_AM:
			return ParamDisplayType::Bool;

		case YM_ALG:
			return ParamDisplayType::MidiChannel;
		}

		return ParamDisplayType::Integer;
	}

private:
	YM2612Param _parameter;
	char _op;
};

class IndexBaron
{
public:
	IndexBaron(void);
	~IndexBaron(void);

	void addIndex(IBIndex* index){_catalogue.push_back(index);}
	IBIndex* getIndex(int index) 
	{ 
		if(index < _catalogue.size())
			return _catalogue[index]; 
		return nullptr;
	}
	IBIndex* getIndexFromMidi(int midi);
	char getCCfromParam(IBIndex* param);
	char getCCRange(IBIndex* param);
	int getIBIndex(IBIndex* index);

	int getYMParamIndex(YM2612Param param, int op = -1);
	int getInsParamIndex(GennyInstrumentParam param);
	int getPatchParamIndex(GennyPatchParam param);

	int getCurrentInstrument() const { return _currentInstrument; }
	void setCurrentInstrument(int ins) { _currentInstrument = ins; }
	int getNumParams() const { return _catalogue.size(); }

	bool enableTrueStereo;

private:
	std::vector<IBIndex*> _catalogue;
	int _currentInstrument;
};

