#pragma once
#include "UIImage.h"
class UICheckbox : public CCheckBox, public GennyInterfaceObject
{
public:
	UICheckbox (const CRect& size, IControlListener* listener, int32_t tag, UTF8StringPtr title, GennyInterface* iface, CBitmap* bitmap = 0, int32_t style = 0);
	~UICheckbox();
	virtual CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseMoved(CPoint& where, const CButtonState& buttons);

	virtual CMouseEventResult onMouseEntered(CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseExited(CPoint& where, const CButtonState& buttons);
	virtual bool onWheel(const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons);

	CLASS_METHODS(UICheckbox, CCheckBox)
	bool controlModifier;

private:
	HMENU ContextMenu;
protected:
	bool _showHints;
};

