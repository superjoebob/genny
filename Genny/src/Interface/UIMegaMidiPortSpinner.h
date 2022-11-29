#pragma once
#include "UIInstrument.h"
#include "UIImage.h"
#include "UICheckBoxNum.h"
#include "Genny2612.h"
class UIInstrumentsPanel;
class UIMegaMidiPortSpinner : public CControl, public IControlListener, public GennyInterfaceObject
{
public:
	UIMegaMidiPortSpinner(CPoint point, GennyInterfaceObject* owner);
	~UIMegaMidiPortSpinner(void);
	virtual bool attached (CView* parent);

	void setValue(float val);
	virtual void valueChanged (CControl* control);
	virtual void draw (CDrawContext* pContext);
	void reconnect();
	void setVisible(bool visible);

	virtual bool onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons);

	CLASS_METHODS(UIMegaMidiPortSpinner, CControl)

private:
	CTextLabel* _midiLabel;
	CKickButton* _upButton;
	CKickButton* _downButton;
	GennyInterfaceObject* _owner;
	CPoint _position;
};

