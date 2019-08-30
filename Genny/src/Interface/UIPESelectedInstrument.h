#pragma once
#include "UIImage.h"
class UIInstrumentsPanel;
struct GennyPatch;
class UIMidiChannel;
class UIPanSlider;
class UIRangeSlider;
class UISpinner;
class UICheckbox;
class UIPESelectedInstrument : public CControlListener, public GennyInterfaceObject
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
	
	bool _selected;
	GennyPatch* _patch;

	UIMidiChannel* _channelSelector;
	UISpinner* _octaveSelector;
	UISpinner* _transposeSelector;

	std::vector<UICheckbox*> _channels;
	CCheckBox* _noiseCheckbox;
	std::vector<UICheckbox*> _fmSelectors;

	UICheckbox* _channel6;
	UIPanSlider* _delaySlider;

	UIPanSlider* _panSlider;
	UIRangeSlider* _rangeSliderLow;
	UIRangeSlider* _rangeSliderHigh;
	CTextLabel* _rangeLabel;

	std::map<int, CControl*> _controls;	
};

