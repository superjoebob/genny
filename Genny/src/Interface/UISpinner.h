#pragma once
#include "UIInstrument.h"
#include "UIImage.h"
#include "UICheckBoxNum.h"
#include "Genny2612.h"
class UIInstrumentsPanel;
class UISpinner : public CControl, public CControlListener, public GennyInterfaceObject
{
public:
	UISpinner(CPoint point, UIInstrumentsPanel* owner, GennyInstrumentParam param);
	~UISpinner(void);
	virtual bool attached (CView* parent);

	void setValue(float val);
	virtual void valueChanged (CControl* control);
	virtual void draw (CDrawContext* pContext);
	void reconnect();

	virtual bool onWheel (const CPoint& where, const float& distance, const CButtonState& buttons);

	CLASS_METHODS(UISpinner, CControl)

private:
	CTextLabel* _midiLabel;
	UIInstrumentsPanel* _owner;
	CPoint _position;

	GennyInstrumentParam _param;
};

