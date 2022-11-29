#include "UISampleRangeSlider.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIPESelectedInstrument.h"
#include "UIDigitDisplay.h"
#include "UIInstrumentsPanel.h"

UISampleRangeSlider::UISampleRangeSlider(const CPoint& pos, int width, UIInstrument* owner, bool low):
	CControl(CRect(), owner),
	GennyInterfaceObject(owner),
	_owner(owner),
	_low(low)
{
	_pos = pos;
	CFrame* frame = getInterface()->getFrame();
	IndexBaron* baron = getIndexBaron();

	UIBitmap sliderKnob(low ? IDB_PNG21 : IDB_PNG22);
	//if(!low)
	//	sliderKnob = UIBitmap(PNG_SLIDERRIGHT);


	_wide = width;

	//setMax(100);

	_low = low;

	
	int index = baron->getInsParamIndex(_low ? GIP_RangeHigh : GIP_RangeLow);
	CRect sliderSize = CRect(0, 0, width , low ? 10 : 14);
	sliderSize.offset(pos.x, pos.y);
	_slider = new UISlider(sliderSize, this, index, pos.x, pos.x + (width - 6), sliderKnob, NULL, getInterface());
	frame->addView(_slider);
	_slider->setBackground(NULL);
	 
	_slider->setMax(GennyInstrumentParam_getRange(_low ? GIP_RangeHigh : GIP_RangeLow));
	_slider->setWheelInc(1 / 127.0f);
	reconnect();
	
}
	
void UISampleRangeSlider::svs(const CRect& r)
{
	_slider->svs(r);

}

void UISampleRangeSlider::sma(const CRect& r)
{
	_slider->setMouseableArea(r);

}


UISampleRangeSlider::~UISampleRangeSlider(void)
{

}

void UISampleRangeSlider::setValue(float val)
{
	_slider->setValue(val);
} 

float UISampleRangeSlider::getValue() const
{
	float realVal = _slider->getValue();
	return realVal;
	
}

void UISampleRangeSlider::valueChanged (CControl* control)
{
	_owner->valueChanged(this);

	WaveData* wave = ((GennyPatch*)getCurrentPatch())->InstrumentDef.Drumset.getDrum(36 + ((GennyPatch*)getCurrentPatch())->InstrumentDef.SelectedDrum);
	if(wave != nullptr)
	{
		if(_low)
			wave->setStartSample((int)control->getValue());
		else
			wave->setEndSample((int)control->getValue());
	}

	float centerPos = (_pos.x + ((_slider->getValue() / (float)_slider->getMax())* _wide)) ;
	_slider->setMouseableArea(CRect(centerPos - 8, _pos.y, centerPos + 8, _pos.y + 10));
}

void UISampleRangeSlider::draw (CDrawContext* pContext)
{
	CControl::setDirty(false);	
}

void UISampleRangeSlider::reconnect()
{
	WaveData* wave = ((GennyPatch*)getCurrentPatch())->InstrumentDef.Drumset.getDrum(36 + ((GennyPatch*)getCurrentPatch())->InstrumentDef.SelectedDrum);
	if(wave != nullptr)
	{
		_slider->setMax((float)wave->originalDataSize);
		_slider->setWheelInc(0.001f);

		if(_low)
			_slider->setValue((float)wave->_originalStartSample);
		else			
			_slider->setValue((float)wave->_originalEndSample);
	}

	float centerPos = (_pos.x + ((_slider->getValue() / (float)_slider->getMax())* _wide)) ;
	_slider->setMouseableArea(CRect(centerPos - 8, _pos.y, centerPos + 8, _pos.y + 10));
}


void UISampleRangeSlider::setVisible(bool visible)
{
	_slider->setVisible(visible);

	if(visible)
		reconnect();
}