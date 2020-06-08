#include "UIOpKnob.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIOperator.h"
#include "UIDigitDisplay.h"

UIOpKnob::UIOpKnob(const CRect& size, CControlListener* listener, long tag, long subPixmaps, CCoord heightOfOneImage, CBitmap* background, GennyInterface* iface, const CPoint& offset):
	UIKnob(size, this, tag, subPixmaps, heightOfOneImage, background, iface, offset)
{
	_realListener = listener; 

	CFrame* frame = iface->getFrame();
	_display = new UIDigitDisplay(CPoint(size.left, size.top + 28), frame, 3, false);
	frame->addView(_display);
}

UIOpKnob::~UIOpKnob(void)
{

}

void UIOpKnob::updateDisplay()
{
	_display->setNumber((int)(getValue() + 0.5f));
}

void UIOpKnob::setValue(float val)
{
	CAnimKnob::setValue(val);
	updateDisplay();
}

void UIOpKnob::valueChanged(CControl* control)
{
	updateDisplay();
	_realListener->valueChanged(control);
}


