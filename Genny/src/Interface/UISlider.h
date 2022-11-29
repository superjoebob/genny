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
	UISlider (const CRect& size, IControlListener* listener, int32_t tag, int32_t iMinPos, int32_t iMaxPos, CBitmap* handle, CBitmap* background, GennyInterface* iface, const CPoint& offset = CPoint (0, 0), const int32_t style = kLeft|kHorizontal);
	~UISlider();
	virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseMoved(CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);

	virtual CMouseEventResult onMouseEntered(CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseExited(CPoint& where, const CButtonState& buttons);

	virtual bool onWheel(const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons);
	virtual void draw (CDrawContext*);

	void svs(const CRect& r);
	float centerStick;
	UISlider* _drawLineTo;
	UISlider* _leftLimiter;
	UISlider* _rightLimiter;
	UISlider* _inputStealer;
	CPoint lineFromOffset;
	CPoint lineToOffset;

	CLASS_METHODS(UISlider, CSlider)


private:
	void updateInputStealer(const CPoint& where);
	GennyInterface* _interface;
	HMENU ContextMenu;
};

