#pragma once
#include "UIInstrument.h"
#include "UIImage.h"
#include "UICheckBoxNum.h"
#include "UIDigitDisplay.h"
#include "YM2612Enum.h"
#include "UIKnob.h"

class UIOperator;
class UIOpKnob : public UIKnob, public CControlListener
{
public:
	UIOpKnob(const CRect& size, CControlListener* listener, long tag, long subPixmaps, CCoord heightOfOneImage, CBitmap* background, GennyInterface* iface, const CPoint& offset = CPoint (0, 0));
	~UIOpKnob(void);
	void setValue(float val);
	void valueChanged(CControl* control);

	void updateDisplay();

	CLASS_METHODS(UIOpKnob, UIKnob)


	UIDigitDisplay* _display;
	CControlListener* _realListener;
};

