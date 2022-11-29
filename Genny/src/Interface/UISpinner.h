#pragma once
#include "UIInstrument.h"
#include "UIImage.h"
#include "UICheckBoxNum.h"
#include "Genny2612.h"
class UIInstrumentsPanel;
class UISpinner : public CControl, public IControlListener, GennyInterfaceObject
{
public:
	UISpinner(CPoint point, GennyInterfaceObject* owner, GennyInstrumentParam param, int op = -1);
	~UISpinner(void);
	virtual bool attached (CView* parent);
	virtual void setVisible(bool visible);

	void setValue(float val);
	virtual void valueChanged (CControl* control);
	virtual void draw (CDrawContext* pContext);
	void reconnect();

	virtual bool onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons);
	virtual CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseMoved(CPoint& where, const CButtonState& buttons);

	virtual CMouseEventResult onMouseEntered(CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseExited(CPoint& where, const CButtonState& buttons);

	CLASS_METHODS(UISpinner, CControl)

private:
	CTextLabel* _midiLabel;
	CKickButton* _upArrow;
	CKickButton* _downArrow;
	GennyInterfaceObject* _owner;
	CPoint _position;
	int _op; 

	GennyInstrumentParam _param;
};

