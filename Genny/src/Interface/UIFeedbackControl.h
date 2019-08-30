#pragma once
#include "UIInstrument.h"
#include "UIImage.h"
#include "UIKnob.h"
class UIFeedbackControl : public CControl, public CControlListener, public GennyInterfaceObject
{
public:
	UIFeedbackControl(const CRect& size, UIInstrument* owner);
	~UIFeedbackControl(void);
	virtual void setValue (float val);
	virtual void valueChanged (CControl* control);
	virtual void draw (CDrawContext* pContext);

	CLASS_METHODS(UIFeedbackControl, CControl)

private:
	UIInstrument* _owner;
	UIKnob* _feedback;
	UIImage* _fbDisplay;
};

