#include "UIReverseKnob.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIOperator.h"
#include "UIDigitDisplay.h"

UIReverseKnob::UIReverseKnob(const CRect& size, CControlListener* listener, long tag, long subPixmaps, CCoord heightOfOneImage, CBitmap* background, GennyInterface* iface, const CPoint& offset):
	UIKnob(size, listener, tag, subPixmaps, heightOfOneImage, background, iface, offset)
{
	//reverse = true;
	bInverseBitmap = true;
}

UIReverseKnob::~UIReverseKnob(void)
{

}

void UIReverseKnob::setValue(float val)
{
	CAnimKnob::setValue(getMax() - val);
}

float UIReverseKnob::getValue() const
{
	return getMax() - CControl::getValue();
}

float UIReverseKnob::getValueNormalized() const
{
	return 1.0f - CControl::getValueNormalized();
}

