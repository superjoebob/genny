#pragma once
#include "UIImage.h"
class UICheckbox : public CCheckBox, public GennyInterfaceObject
{
public:
	UICheckbox (const CRect& size, CControlListener* listener, int32_t tag, UTF8StringPtr title, GennyInterface* iface, CBitmap* bitmap = 0, int32_t style = 0);
	~UICheckbox();
	virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);
	CLASS_METHODS(UICheckbox, CCheckBox)

private:
	HMENU ContextMenu;
};

