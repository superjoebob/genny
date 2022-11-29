#include "UISlider.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIContextMenu.h"
#include "lib/platform/win32/win32frame.h"

UISlider::UISlider (const CRect& size, IControlListener* listener, int32_t tag, int32_t iMinPos, int32_t iMaxPos, CBitmap* handle, CBitmap* background, GennyInterface* iface, const CPoint& offset, const int32_t style):
	CSlider(size, listener, tag, iMinPos, iMaxPos, handle, background, offset, style),
	GennyInterfaceObject(iface),
	_drawLineTo(nullptr),
	_leftLimiter(nullptr),
	_rightLimiter(nullptr),
	_inputStealer(nullptr),
	lineFromOffset(CPoint()),
	lineToOffset(CPoint())
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
	if (_inputStealer != nullptr)
	{
		CMouseEventResult res = _inputStealer->onMouseUp(where, buttons);
		_inputStealer = nullptr;
		return res;
	}

	if (buttons.isRightButton())
	{
		onMouseUpContext(tag);
		return kMouseEventHandled;
	}
	else
	{
		if (centerStick >= 0.0f)
		{
			if (abs(value - centerStick) < (getMax() * 0.05f))
			{
				setValue(centerStick);
				valueChanged();
			}
		}

		return CSlider::onMouseUp(where, buttons);
	}
}

CMouseEventResult UISlider::onMouseMoved(CPoint& where, const CButtonState& buttons)
{
	if (_inputStealer != nullptr)
		return _inputStealer->onMouseMoved(where, buttons);

	if (buttons.isRightButton())
		return kMouseEventHandled;

	CMouseEventResult res = __super::onMouseMoved(where, buttons);
	if (buttons.isLeftButton() && centerStick >= 0.0f)
	{
		//float val = value;
		if (abs(value - centerStick) < (getMax() * 0.05f))
		{
			value = centerStick;
			valueChanged();
		}
		//value = val;
	}
	return res;
}

CMouseEventResult UISlider::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	updateInputStealer(where);
	if(_inputStealer != nullptr)
		return _inputStealer->onMouseDown(where, buttons);

	if(buttons.isRightButton())
		return kMouseEventHandled;
	else 
		return CSlider::onMouseDown(where, buttons);
}

CMouseEventResult UISlider::onMouseEntered(CPoint& where, const CButtonState& buttons)
{
	getInterface()->hoverControl(this);
	return __super::onMouseEntered(where, buttons);
}

CMouseEventResult UISlider::onMouseExited(CPoint& where, const CButtonState& buttons)
{
	getInterface()->unhoverControl(this);
	return __super::onMouseExited(where, buttons);
}

bool UISlider::onWheel(const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons)
{
	updateInputStealer(where);
	if (_inputStealer != nullptr)
	{
		bool res = _inputStealer->onWheel(where, axis, distance, buttons);
		_inputStealer = nullptr;
		return res;
	}

	return __super::onWheel(where, axis, distance, buttons);
}

void UISlider::updateInputStealer(const CPoint& where)
{
	if (_leftLimiter != nullptr)
	{
		if (where.x < handlePos.getCenter().x && (where.x < _leftLimiter->handlePos.getCenter().x || (abs(where.x - _leftLimiter->handlePos.getCenter().x) < abs(where.x - handlePos.getCenter().x))))
			_inputStealer = _leftLimiter;
		//return CMouseEventResult::kMouseEventNotHandled;
	}
	else if (_rightLimiter != nullptr)
	{
		if (where.x > handlePos.getCenter().x && (where.x > _rightLimiter->handlePos.getCenter().x || (abs(where.x - _rightLimiter->handlePos.getCenter().x) < abs(where.x - handlePos.getCenter().x))))
			_inputStealer = _leftLimiter;
	}
}

void UISlider::draw (CDrawContext* c)
{
	CSlider::draw(c);
	if (_drawLineTo != nullptr)
	{
		c->setFrameColor(CColor(80, 133, 71, 255));
		//c->setFrameColor(CColor(80, 33, 71, 255));
		c->setLineStyle(kLineSolid);
		c->setLineWidth(2);

		handlePos = calculateHandleRect(getValueNormalized());
		_drawLineTo->handlePos = _drawLineTo->calculateHandleRect(_drawLineTo->getValueNormalized());
		c->drawLine((handlePos.getCenter() + CPoint(0, 4)) + lineFromOffset, (_drawLineTo->handlePos.getCenter() + CPoint(0, -4)) + lineToOffset);
	}
}