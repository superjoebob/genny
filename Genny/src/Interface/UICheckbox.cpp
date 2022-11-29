#include "UICheckbox.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIContextMenu.h"
#include "lib/platform/win32/win32frame.h"

UICheckbox::UICheckbox (const CRect& size, IControlListener* listener, int32_t tag, UTF8StringPtr title, GennyInterface* iface, CBitmap* bitmap, int32_t style)
	:CCheckBox(size, listener, tag, title, bitmap, style),
	 GennyInterfaceObject(iface),
	 _showHints(true),
	controlModifier(false)
{
	this->setWheelInc(1.0f);
	ContextMenu = CreatePopupMenu();
}

UICheckbox::~UICheckbox()
{
	// destroy our popup menu and all subitems
	int count = GetMenuItemCount(ContextMenu);
	while (count) {
		DeleteMenu(ContextMenu, count-1, MF_BYPOSITION);
		count--;
	}
	DestroyMenu(ContextMenu);
}


CMouseEventResult UICheckbox::onMouseUp(CPoint& where, const CButtonState& buttons)
{
	if (buttons.getModifierState() == CButton::kControl)
		controlModifier = true;
	else
		controlModifier = false;


	if (buttons.isRightButton())
	{
		onMouseUpContext(tag);
		return kMouseEventHandled;
	}
	else
	{
		return CCheckBox::onMouseUp(where, buttons);
	}
}

CMouseEventResult UICheckbox::onMouseMoved(CPoint& where, const CButtonState& buttons)
{
	if (buttons.isRightButton())
		return kMouseEventHandled;

	return __super::onMouseMoved(where, buttons);
}

CMouseEventResult UICheckbox::onMouseDown(CPoint& where, const CButtonState& buttons)
{
	if (buttons.isRightButton())
		return kMouseEventHandled;
	else
		return CCheckBox::onMouseDown(where, buttons);
}

CMouseEventResult UICheckbox::onMouseEntered(CPoint& where, const CButtonState& buttons)
{
	if(_showHints)
		getInterface()->hoverControl(this);

	return __super::onMouseEntered(where, buttons);
}

CMouseEventResult UICheckbox::onMouseExited(CPoint& where, const CButtonState& buttons)
{
	if (_showHints)
		getInterface()->unhoverControl(this);

	return __super::onMouseExited(where, buttons);
}

bool UICheckbox::onWheel(const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons)
{
	return __super::onWheel(where, axis, distance, buttons);
}
