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
class UIRangeSlider;
class UICheckbox;
class UIPanSlider;
class UISlider;
class UIDigitKnob;
class UIKnob;
class UIInstrument : public IControlListener, public GennyInterfaceObject
{
public:
	UIInstrument(GennyInterface* owner);
	~UIInstrument(void);

	CFrame* getFrame() { return _owner->getFrame(); }
	IndexBaron* getIndexBaron() { return _owner->getIndexBaron(); }
	void initialize();
	void setParam(int index, float val);
	virtual void valueChanged (CControl* control);
	virtual int32_t controlModifierClicked(VSTGUI::CControl* pControl, VSTGUI::CButtonState button);

	virtual void midiLearn(int index);
	virtual void midiForget(int index);
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

	UIDigitKnob* _dacSamplerate;
	UIChannelStatus* _channelStatus;
	std::vector<UIOperator*> _operators;
	UIDigitDisplay* _pitchDigits;
	int _pitchIndex;
	float _lastDrumClick;
	int _lastDrumClickTag;


	CTextLabel* _drumSampleLowLabel;
	CTextLabel* _drumSampleHighLabel;
	
	UIPanSlider* _samplePitchSlider;
	UIRangeSlider* _sampleRangeLow;
	UIRangeSlider* _sampleRangeHigh;
	UISlider* _sampleVolumeSlider;
	UIImage* _sampleVolumeBacker;
	UIImage* _drumSettingsCover;
	UICheckbox* _sampleLoop;
	UICheckbox* _ch3SpecialBox;
	UICheckbox* _lfoEnCheckbox;
	UIDigitKnob* _lfoKnob;


	UICheckbox* _drumChL;
	UICheckbox* _drumChR;

	CCheckBox* _specialMode;
	std::vector<CKickButton*> _drumButtons;
	std::vector<UIImage*> _drumLights;
	std::vector<float> _drumLightVals;

	int rangeTop;
	int rangeBottom;
};

