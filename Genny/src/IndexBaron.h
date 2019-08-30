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
	IBIndex(std::string name = "") :_name(name) { }
	IBType getType() const { return _type; }
	std::string getName() const { return _name; }

	virtual std::string getValue(GennyPatch* patch) { return ""; }

protected:
	IBType _type;
	std::string _name;
};


class IBPatchParam : public IBIndex
{
public:
	IBPatchParam(GennyPatchParam param, std::string name = "")
		:IBIndex(name)
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
	IBInsParam(char instrument, std::string name = "")
		:IBIndex(name)
	    ,_instrument(instrument)
		,_parameter(GIP_None)
	{ 
		_type = IB_InsParam; 
	}

	IBInsParam(char instrument, GennyInstrumentParam param, std::string name = "")
		:IBIndex(name)
	    ,_instrument(instrument)
		,_parameter(param)
	{ 
		_type = IB_InsParam; 
	}

	char getInstrument() const { return _instrument; }
	GennyInstrumentParam getParameter() const { return _parameter; }

protected:
	char _instrument;

private:
	GennyInstrumentParam _parameter;
};


class IBYMParam : public IBInsParam
{
public:
	IBYMParam(YM2612Param param, char op, char instrument, std::string name = "")
	    :IBInsParam(instrument, name)
		,_parameter(param)
		,_op(op)
	{ 
		_type = IB_YMParam; 
	}

	YM2612Param getParameter() const { return _parameter; }
	char getOperator() const { return _op; }
	virtual std::string getValue(GennyPatch* patch);

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
	IBIndex* getIndex(int index) { return _catalogue[index]; }
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

private:
	std::vector<IBIndex*> _catalogue;
	int _currentInstrument;
};

