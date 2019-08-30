#include "UICheckbox.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIContextMenu.h"
#include "lib/platform/win32/win32frame.h"

UICheckbox::UICheckbox (const CRect& size, CControlListener* listener, int32_t tag, UTF8StringPtr title, GennyInterface* iface, CBitmap* bitmap, int32_t style)
	:CCheckBox(size, listener, tag, title, bitmap, style),
	 GennyInterfaceObject(iface)
{
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


CMouseEventResult UICheckbox::onMouseUp (CPoint& where, const CButtonState& buttons)
{
#if !BUILD_VST
	
	//if(_interface != nullptr)
	//{
	//	VSTBase* b = getVst()->getBase(); 
	//	int col = -1;
	//	char* result = new char[255];
	//	//b->PlugHost->PromptEdit(-1, -1, "BABA", result, col); 
	//	//b->PlugHost->Dispatcher(b->HostTag, FHD_SetNewColor, 0, 5);
	//}
	
	if(_interface != nullptr && buttons.isRightButton())
	{
		VSTBase* b = getVst()->getBase(); 

		Win32Frame* winFrame = (Win32Frame*)_interface->getFrame()->getPlatformFrame();
		CPoint mousePos, globalPos;
		winFrame->getCurrentMousePosition(mousePos);
		winFrame->getGlobalPosition(globalPos);
		mousePos = mousePos + globalPos;

		int numParams = GennyPatch::getNumParameters();
		int patchNum = getVst()->getPatchIndex(getCurrentPatch());
		int paramTag = (numParams * patchNum) + tag;
		
		int param = 0;
		b->AdjustParamPopup(ContextMenu, paramTag, 0, DefaultMenuID);

		BOOL r = TrackPopupMenu(ContextMenu, TPM_RETURNCMD | TPM_RIGHTBUTTON, mousePos.x, mousePos.y, 0, winFrame->getPlatformWindow() , NULL);
		if (r) 
			b->PlugHost->Dispatcher(b->HostTag, FHD_ParamMenu, paramTag, r-DefaultMenuID);

		return kMouseEventHandled;
	}
	else
#endif
		return CCheckBox::onMouseUp(where, buttons);
}

CMouseEventResult UICheckbox::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if(_interface != nullptr && buttons.isRightButton())
		return kMouseEventHandled;
	else 
		return CCheckBox::onMouseDown(where, buttons);
}