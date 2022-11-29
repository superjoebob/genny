#pragma once
#include "UIInstrument.h"
#include "UIImage.h"
#include "UICheckBoxNum.h"
#include "Genny2612.h"
class UIInstrument;
class UIWaveForm : public CControl, public IControlListener, public GennyInterfaceObject
{
public:
	UIWaveForm(CPoint point, UIInstrument* owner);
	~UIWaveForm(void);
	virtual CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons);

	void setValue(float val);
	virtual void valueChanged (CControl* control);
	virtual void draw (CDrawContext* pContext);
	void reconnect();

	CLASS_METHODS(UIWaveForm, CControl)
		 
private:
	int _xpos;  
	int _ypos;
};

