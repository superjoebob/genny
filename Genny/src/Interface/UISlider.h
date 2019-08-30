#pragma once
#include "UIInstrument.h"
#include "UIImage.h"
#include "UICheckBoxNum.h"
#include "UIDigitDisplay.h"
#include "YM2612Enum.h"
#include "Genny2612.h"
class UISlider : public CSlider, public GennyInterfaceObject
{
public:
	UISlider (const CRect& size, CControlListener* listener, int32_t tag, int32_t iMinPos, int32_t iMaxPos, CBitmap* handle, CBitmap* background, GennyInterface* iface, const CPoint& offset = CPoint (0, 0), const int32_t style = kLeft|kHorizontal);
	~UISlider();
	virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);
	virtual void draw (CDrawContext*);

	void svs(const CRect& r);
	float centerStick;

	CLASS_METHODS(UISlider, CSlider)

private:
	GennyInterface* _interface;
	HMENU ContextMenu;
};

