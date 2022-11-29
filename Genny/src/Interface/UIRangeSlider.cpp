#include "UIRangeSlider.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIPESelectedInstrument.h"
#include "UIDigitDisplay.h"
#include "UIInstrumentsPanel.h"

UIRangeSlider::UIRangeSlider(const CPoint& vPosition, int vWidth, GennyInterfaceObject* vOwner, IControlListener* listener, bool vLow, bool flipRight):
	CControl(CRect(), listener),
	GennyInterfaceObject(vOwner),
	_low(vLow),
	_width(vWidth),
	_position(vPosition),
	_flipRight(flipRight)
{

}

UIRangeSlider::~UIRangeSlider(void)
{

}

bool UIRangeSlider::attached (CView* parent)
{
	bool returnValue = CControl::attached(parent);

	CFrame* frame = getFrame();
	IndexBaron* baron = getIndexBaron();

	UIBitmap sliderKnob(_flipRight ? PNG_SAMPLESLIDERKNOB : (_low ? PNG_SLIDERLEFT : PNG_SLIDERRIGHT));

	setMax(100);
	
	int index = baron->getInsParamIndex(_low ? GIP_RangeHigh : GIP_RangeLow);
	CRect sliderSize = CRect(0, 0, _width + 2, 14);
	sliderSize.offset(_position.x, _position.y);
	_slider = new UISlider(sliderSize, this, index, _position.x, _position.x + (_width - (_flipRight ? 18 : 16)), sliderKnob, NULL, getInterface());
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
	listener->valueChanged(this);
	setDirty(true);
}

void UIRangeSlider::draw (CDrawContext* pContext)
{
	CControl::setDirty(false);
}

void UIRangeSlider::setVisible(bool visible)
{
	__super::setVisible(visible);
	_slider->setVisible(visible);
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
