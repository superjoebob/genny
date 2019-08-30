#include "UIKnob.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIContextMenu.h"
#include "lib/platform/win32/win32frame.h"

UIKnob::UIKnob (const CRect& size, CControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, CBitmap* background, GennyInterface* iface, const CPoint& offset):
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
	while (count) {
		DeleteMenu(ContextMenu, count-1, MF_BYPOSITION);
		count--;
	}
	DestroyMenu(ContextMenu);
}

CMouseEventResult UIKnob::onMouseUp (CPoint& where, const CButtonState& buttons)
{
#if !BUILD_VST
	if(buttons.isRightButton())
	{
		VSTBase* b = getVst()->getBase();

		Win32Frame* winFrame = (Win32Frame*)_interface->getFrame()->getPlatformFrame();
		CPoint mousePos, globalPos;
		winFrame->getCurrentMousePosition(mousePos);
		winFrame->getGlobalPosition(globalPos);
		mousePos = mousePos + globalPos;

		
		GennyPatch* patch0 = static_cast<GennyPatch*>(getVst()->getPatch(0));
		int numParams = GennyPatch::getNumParameters();
		int patchNum = patch0->SelectedInstrument;
		int paramTag = (numParams * patchNum) + tag;

		b->AdjustParamPopup(ContextMenu, paramTag, 0, DefaultMenuID);
		BOOL r = TrackPopupMenu(ContextMenu, TPM_RETURNCMD | TPM_RIGHTBUTTON, mousePos.x, mousePos.y, 0, winFrame->getPlatformWindow() , NULL);
		if (r)
		{
			int val = b->PlugHost->Dispatcher(b->HostTag, FHD_ParamMenu, paramTag, r-DefaultMenuID);
			int qq = 1;
		}

		return kMouseEventHandled;
	}
	else
#endif
		return CKnob::onMouseUp(where, buttons);
}

CMouseEventResult UIKnob::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if(buttons.isRightButton())
		return kMouseEventHandled;
	else 
		return CKnob::onMouseDown(where, buttons);
}