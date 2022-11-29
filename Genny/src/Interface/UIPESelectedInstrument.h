#pragma once
#include "UIImage.h"
class UIInstrumentsPanel;
struct GennyPatch;
class UIMidiChannel;
class UIPanSlider;
class UIRangeSlider;
class UISpinner;
class UICheckbox;
class UIPESelectedInstrument : public IControlListener, public GennyInterfaceObject
{
public:
	UIPESelectedInstrument(UIInstrumentsPanel* vOwner, GennyPatch* vPatch, int vIndex);
	~UIPESelectedInstrument(void);

	virtual void valueChanged (CControl* control);
	void setPatchLink(GennyPatch* patch);
	GennyPatch* getPatchLink() { return _patch; }
	void reconnect();

	void mapControl(CControl* control, int index) { _controls[index] = control; }
	UIInstrumentsPanel* getOwner() {return _owner;}

private:
	UIInstrumentsPanel* _owner;
	CTextLabel* _label;
	
	bool _togglingChannels;
	bool _selected;
	GennyPatch* _patch;

	UIMidiChannel* _channelSelector;
	UISpinner* _octaveSelector;
	UISpinner* _transposeSelector;
	UIImage* _transTab;
	CKickButton* _transTabButton;
	CKickButton* _moreTabButton;
	CKickButton* _delayTabButton;
	CCheckBox* _soloCheckbox;
	CCheckBox* _legatoCheckbox;

	CCheckBox* _lChannelButton;
	CCheckBox* _rChannelButton;

	CKickButton* _pingPongButton;

	std::vector<UICheckbox*> _channels;
	CCheckBox* _noiseCheckbox;
	std::vector<UICheckbox*> _fmSelectors;

	UICheckbox* _channel6;
	UICheckbox* _channelSp3;
	UIPanSlider* _delaySlider;
	UIPanSlider* _glideSlider;

	UIPanSlider* _panSlider;
	UIPanSlider* _detuneSlider;
	UIRangeSlider* _rangeSliderLow;
	UIRangeSlider* _rangeSliderHigh;
	CTextLabel* _rangeLabel;
	CTextLabel* _pingpongLabel;

	std::map<int, CControl*> _controls;	
};

