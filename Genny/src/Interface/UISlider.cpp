#include "UISlider.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIContextMenu.h"
#include "lib/platform/win32/win32frame.h"

UISlider::UISlider (const CRect& size, CControlListener* listener, int32_t tag, int32_t iMinPos, int32_t iMaxPos, CBitmap* handle, CBitmap* background, GennyInterface* iface, const CPoint& offset, const int32_t style):
	CSlider(size, listener, tag, iMinPos, iMaxPos, handle, background, offset, style),
	GennyInterfaceObject(iface)
{
	_interface = iface;
	ContextMenu = CreatePopupMenu();
	centerStick = -1.0f;
}
UISlider::~UISlider()
{
	// destroy our popup menu and all subitems
	int count = GetMenuItemCount(ContextMenu);
	while (count) {
		DeleteMenu(ContextMenu, count-1, MF_BYPOSITION);
		count--;
	}
	DestroyMenu(ContextMenu);
}

void UISlider::svs(const CRect& r)
{
	setViewSize(r, true);
}

CMouseEventResult UISlider::onMouseUp (CPoint& where, const CButtonState& buttons)
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
			b->PlugHost->Dispatcher(b->HostTag, FHD_ParamMenu, paramTag, r-DefaultMenuID);

		return kMouseEventHandled;
	}
	else
#endif

	if(centerStick >= 0.0f)
	{
		if(abs(value - centerStick) < (getMax() * 0.05f))
		{
			setValue(centerStick);
			valueChanged();
		}
	}
		
	return CSlider::onMouseUp(where, buttons);
}

CMouseEventResult UISlider::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if(buttons.isRightButton())
		return kMouseEventHandled;
	else 
		return CSlider::onMouseDown(where, buttons);
}
	
void UISlider::draw (CDrawContext* c)
{
	if(centerStick >= 0.0f)
	{
		float val = value;
		if(abs(value - centerStick) < (getMax() * 0.05f))
			value = centerStick;
		CSlider::draw(c);
		value = val;
	}
	else
		CSlider::draw(c);
}