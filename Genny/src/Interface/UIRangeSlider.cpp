#include "UIRangeSlider.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIPESelectedInstrument.h"
#include "UIDigitDisplay.h"
#include "UIInstrumentsPanel.h"

UIRangeSlider::UIRangeSlider(const CPoint& vPosition, int vWidth, UIPESelectedInstrument* vOwner, bool vLow):
	CControl(CRect(), vOwner),
	GennyInterfaceObject(vOwner),
	_owner(vOwner),
	_low(vLow),
	_width(vWidth),
	_position(vPosition)
{

}

UIRangeSlider::~UIRangeSlider(void)
{

}

bool UIRangeSlider::attached (CView* parent)
{
	bool returnValue = CControl::attached(parent);

	CFrame* frame = _owner->getOwner()->getFrame();
	IndexBaron* baron = getIndexBaron();

	UIBitmap sliderKnob(_low ? PNG_SLIDERRIGHT : PNG_SLIDERLEFT);

	setMax(100);
	
	int index = baron->getInsParamIndex(_low ? GIP_RangeHigh : GIP_RangeLow);
	CRect sliderSize = CRect(0, 0, _width , _low ? 10 : 14);
	sliderSize.offset(_position.x, _position.y);
	_slider = new UISlider(sliderSize, this, index, _position.x, _position.x + (_width - 16), sliderKnob, NULL, getInterface());
	frame->addView(_slider);
	_slider->setBackground(NULL);

	_slider->setMax(GennyInstrumentParam_getRange(_low ? GIP_RangeHigh : GIP_RangeLow));
	_slider->setWheelInc(1 / 127.0f);
	reconnect();

	return returnValue;
}

void UIRangeSlider::setValue(float val)
{
	_slider->setValue(val);
} 

float UIRangeSlider::getValue() const
{
	float realVal = _slider->getValue();
	return realVal;
	
}

void UIRangeSlider::valueChanged (CControl* control)
{
	_owner->valueChanged(this);
}

void UIRangeSlider::draw (CDrawContext* pContext)
{
	CControl::setDirty(false);
}

void UIRangeSlider::reconnect()
{
	IndexBaron* baron = getIndexBaron();

	int index = baron->getPatchParamIndex(GPP_SelectedInstrument);

	int selection = getInstrumentIndex(getPatch(0)->SelectedInstrument);
	GennyPatch* selectedPatch = (GennyPatch*)getVst()->getPatch(selection);


	index = baron->getInsParamIndex(_low ? GIP_RangeHigh : GIP_RangeLow);
	setValue(selectedPatch->getFromBaron(baron->getIndex(index)));
	tag = (_low ? kRangeHighStart : kRangeLowStart) + selection;
	_slider->setTag(index);
}
