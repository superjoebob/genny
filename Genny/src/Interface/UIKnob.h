#pragma once
#include "UIInstrument.h"
#include "UIImage.h"
#include "UICheckBoxNum.h"
#include "UIDigitDisplay.h"
#include "YM2612Enum.h"
#include "Genny2612.h"
class UIOperator;
class UIKnob : public CAnimKnob, public GennyInterfaceObject
{
public:
	UIKnob (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, CBitmap* background, GennyInterface* iface, const CPoint& offset = CPoint (0, 0));
	~UIKnob();
	virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseMoved(CPoint& where, const CButtonState& buttons);

	virtual CMouseEventResult onMouseEntered(CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseExited(CPoint& where, const CButtonState& buttons);

	virtual void setValue(float val);
	CLASS_METHODS(UIKnob, CAnimKnob)


private:
	GennyInterface* _interface;
	HMENU ContextMenu;
};

