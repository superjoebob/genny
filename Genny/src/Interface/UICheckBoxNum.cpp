#include "UICheckBoxNum.h"
#include "GennyInterface.h"
#include "UIAlgorithmSelector.h"
#include "../resource.h"

UICheckBoxNum::UICheckBoxNum(const CRect& size, int num, IControlListener* listener, long tag, CBitmap* bitmap, GennyInterface* ins, bool special, const long style, int topoff, UIAlgorithmSelector* algSelector):
	UICheckbox(size, listener, tag, "", ins, bitmap, style),
	_num(num),
	_special(special),
	_topOff(topoff),
	_algSelector(algSelector)
{
	

}
	
UICheckBoxNum::~UICheckBoxNum()
{	
}


CMouseEventResult UICheckBoxNum::onMouseUp(CPoint& where, const CButtonState& buttons)
{
	if (buttons.isRightButton() && _algSelector != nullptr)
	{
		_algSelector->onMouseUpContext(_algSelector->getTag());
		return kMouseEventHandled;
	}

	__super::onMouseUp(where, buttons);
}

CMouseEventResult UICheckBoxNum::onMouseEntered(CPoint& where, const CButtonState& buttons)
{
	if(_algSelector != nullptr)
		getInterface()->hoverControl(_algSelector);

	return __super::onMouseEntered(where, buttons);
}

CMouseEventResult UICheckBoxNum::onMouseExited(CPoint& where, const CButtonState& buttons)
{
	if (_algSelector != nullptr)
		getInterface()->unhoverControl(_algSelector);

	return __super::onMouseExited(where, buttons);
}

bool UICheckBoxNum::onWheel(const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons)
{
	if (_algSelector != nullptr)
	{
		_algSelector->onWheel(where, axis, distance, buttons);
		return true;
	}

	return __super::onWheel(where, axis, distance, buttons);
}


bool UICheckBoxNum::attached (CView* parent)
{
	CRect size = getViewSize();
	bool returnValue = UICheckbox::attached(parent);
	
	if(_special) 
	{
		CRect pos = CRect( 0, 0, 12, 14 );
		if (size.getWidth() > 60)
			pos.offset(size.left + 46, size.top + 2 + _topOff);
		else if(size.getWidth() > 20)
			pos.offset(size.left + 20, size.top + 2 + _topOff);
		else
			pos.offset(size.left + 2, size.top + 2 + _topOff);

		_label = new UIImage( pos, PNG_INSTRUMENTNUMBERS );
	}
	else
	{
		CRect pos = CRect( 0, 0, 12, 12 );
		pos.offset(size.left + 8, size.top + 8 + _topOff);
		_label = new UIImage( pos, PNG_BUTTONNUMBERS );
	}
	_label->setFrame(_num);
	_interface->getFrame()->addView(_label);
	_label->setMouseableArea(CRect());

	if (_algSelector != nullptr)
		_showHints = false;

	return returnValue;
}
	
void UICheckBoxNum::setVisible(bool visible)
{
	_label->setVisible(visible);
	__super::setVisible(visible);
}
