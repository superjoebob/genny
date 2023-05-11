#include "UIKnob.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIContextMenu.h"
#include "lib/platform/win32/win32frame.h"

UIKnob::UIKnob (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, CBitmap* background, GennyInterface* iface, const CPoint& offset):
	CAnimKnob(size, listener, tag, subPixmaps, heightOfOneImage, background, offset),
	GennyInterfaceObject(iface)
{
	_interface = iface;
	ContextMenu = CreatePopupMenu();
}

UIKnob::~UIKnob()
{
	// destroy our popup menu and all subitems
	int count = GetMenuItemCount(ContextMenu);
	while (count > 0) {
		DeleteMenu(ContextMenu, count-1, MF_BYPOSITION);
		count--;
	}
	DestroyMenu(ContextMenu);
}

CMouseEventResult UIKnob::onMouseUp (CPoint& where, const CButtonState& buttons)
{

	if (buttons.isRightButton())
	{
		onMouseUpContext(tag);
		return kMouseEventHandled;
	}
	return __super::onMouseUp(where, buttons);
}

CMouseEventResult UIKnob::onMouseMoved(CPoint& where, const CButtonState& buttons)
{
	if (buttons.isRightButton())
		return kMouseEventHandled;

	return __super::onMouseMoved(where, buttons);
}

CMouseEventResult UIKnob::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if(buttons.isRightButton())
		return kMouseEventHandled;
	else 
		return __super::onMouseDown(where, buttons);
}

CMouseEventResult UIKnob::onMouseEntered(CPoint& where, const CButtonState& buttons)
{
	getInterface()->hoverControl(this);
	return __super::onMouseEntered(where, buttons);
}

CMouseEventResult UIKnob::onMouseExited(CPoint& where, const CButtonState& buttons)
{
	getInterface()->unhoverControl(this);
	return __super::onMouseExited(where, buttons);
}

void UIKnob::setValue(float val)
{
	//if (directParent != nullptr)
	//	directParent->childValueChanged();

	CAnimKnob::setValue(val);
}