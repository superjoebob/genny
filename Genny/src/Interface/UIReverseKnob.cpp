#include "UIReverseKnob.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIOperator.h"
#include "UIDigitDisplay.h"

UIReverseKnob::UIReverseKnob(const CRect& size, IControlListener* listener, long tag, long subPixmaps, CCoord heightOfOneImage, CBitmap* background, GennyInterface* iface, const CPoint& offset):
	UIKnob(size, this, tag, subPixmaps, heightOfOneImage, background, iface, offset)
{
	_realListener = listener;
	//reverse = true;
	bInverseBitmap = true;

	CFrame* frame = iface->getFrame();
	_display = new UIDigitDisplay(CPoint(size.left, size.top + 28), frame, 3, false);
	frame->addView(_display);
}

UIReverseKnob::~UIReverseKnob(void)
{

}

void UIReverseKnob::updateDisplay()
{
	_display->setNumber((int)((getMax() - getValue()) + 0.5f));
}

void UIReverseKnob::setValue(float val)
{
	CAnimKnob::setValue(getMax() - val);
	updateDisplay();
}

float UIReverseKnob::getValue() const
{
	return getMax() - CControl::getValue();
}

float UIReverseKnob::getValueNormalized() const
{
	return 1.0f - CControl::getValueNormalized();
}

void UIReverseKnob::valueChanged(CControl* control)
{
	updateDisplay();
	_realListener->valueChanged(control);
}


