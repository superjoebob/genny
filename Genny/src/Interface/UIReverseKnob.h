#pragma once
#include "UIInstrument.h"
#include "UIImage.h"
#include "UICheckBoxNum.h"
#include "UIDigitDisplay.h"
#include "YM2612Enum.h"
#include "UIKnob.h"

class UIOperator;
class UIReverseKnob : public UIKnob
{
public:
	UIReverseKnob(const CRect& size, CControlListener* listener, long tag, long subPixmaps, CCoord heightOfOneImage, CBitmap* background, GennyInterface* iface, const CPoint& offset = CPoint (0, 0));
	~UIReverseKnob(void);
	void setValue(float val);
	float getValue() const;
	float getValueNormalized() const;

	CLASS_METHODS(UIReverseKnob, UIKnob)
};

