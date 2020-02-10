#pragma once
#include <map>
#include <vector>
#include "GennyInterface.h"

class GennyInterface;
class IBYMParam;
class UIChannelStatus;
class UIOperator;
class UIDigitDisplay;
class UIImage;
class UIWaveForm;
class UISampleRangeSlider;
class UICheckbox;
class UIInstrument : public CControlListener, public GennyInterfaceObject
{
public:
	UIInstrument(GennyInterface* owner);
	~UIInstrument(void);

	CFrame* getFrame() { return _owner->getFrame(); }
	IndexBaron* getIndexBaron() { return _owner->getIndexBaron(); }
	void initialize();
	void setParam(int index, float val);
	virtual void valueChanged (CControl* control);

	void reconnect();

	void makeChannelsDirty();
	void updateChannels();

	void mapControl(CControl* control, int index);

	void addRefreshable(CView* refresh) { _refreshList.push_back(refresh); }
	void makeOperatorDirty(int op);

	GennyInterface* getOwner() { return _owner; }

private:
	GennyInterface* _owner;
	std::map<int, CControl*> _controls;	
	std::map<int, CControl*> _globalControls;	
	std::vector<CView*> _doors;
	UIWaveForm* _wave;


	std::vector<CView*> _snControls;
	std::vector<CView*> _drumControls;
	std::vector<CView*> _refreshList;

	UIImage* _drumDoor;
	UIImage* _SNDoor;
	CTextLabel* _drumLabel;

	UIChannelStatus* _channelStatus;
	std::vector<UIOperator*> _operators;
	UIDigitDisplay* _pitchDigits;
	int _pitchIndex;
	float _lastDrumClick;



	UISampleRangeSlider* _sampleRangeLow;
	UISampleRangeSlider* _sampleRangeHigh;
	UICheckbox* _sampleNormalize;
	UICheckbox* _sampleLoop;
	CCheckBox* _specialMode;
	std::vector<CKickButton*> _drumButtons;
	std::vector<UIImage*> _drumLights;
	std::vector<float> _drumLightVals;

	int rangeTop;
	int rangeBottom;
};

