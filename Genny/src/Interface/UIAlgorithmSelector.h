#pragma once
#include "UIInstrument.h"
#include "UIImage.h"
#include "UICheckBoxNum.h"
class UIAlgorithmSelector : public CControl, public CControlListener, public GennyInterfaceObject
{
public:
	UIAlgorithmSelector(const CRect& size, UIInstrument* owner);
	~UIAlgorithmSelector(void);
	void setValue(float val);
	virtual void valueChanged (CControl* control);
	virtual void draw (CDrawContext* pContext);

	CLASS_METHODS(UIAlgorithmSelector, CControl)

private:
	UIInstrument* _owner;
	std::vector<UICheckBoxNum*> _algs;
	UIImage* _algDisplay;
};

