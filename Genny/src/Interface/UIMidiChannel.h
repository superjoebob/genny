#pragma once
#include "UIImage.h"
class UIInstrumentsPanel;
class UIMidiChannel : public CControl, public IControlListener, GennyInterfaceObject
{
public:
	UIMidiChannel(const CPoint& locasticon, GennyInterfaceObject* owner, IControlListener* listener, int op);
	~UIMidiChannel(void);
	virtual bool attached (CView* parent);

	void setValue(float val);
	virtual void valueChanged (CControl* control);
	virtual void draw (CDrawContext* pContext);
	virtual void setVisible(bool visible);
	void reconnect();
	
	virtual bool onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons);

	virtual CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseMoved(CPoint& where, const CButtonState& buttons);

	virtual CMouseEventResult onMouseEntered(CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseExited(CPoint& where, const CButtonState& buttons);

	CLASS_METHODS(UIMidiChannel, CControl)

private:
	CTextLabel* _midiLabel;
	CKickButton* _upArrow;
	CKickButton* _downArrow;
	int _op;
};

