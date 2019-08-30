#pragma once
#include "UIInstrument.h"
#include "UIImage.h"
#include "UICheckBoxNum.h"
class UIInstrumentsPanel;
class UIMidiChannel : public CControl, public CControlListener, GennyInterfaceObject
{
public:
	UIMidiChannel(UIInstrumentsPanel* owner);
	~UIMidiChannel(void);
	virtual bool attached (CView* parent);

	void setValue(float val);
	virtual void valueChanged (CControl* control);
	virtual void draw (CDrawContext* pContext);
	void reconnect();
	
	virtual bool onWheel (const CPoint& where, const float& distance, const CButtonState& buttons);

	CLASS_METHODS(UIMidiChannel, CControl)

private:
	UIInstrumentsPanel* _owner;
	CTextLabel* _midiLabel;
};

