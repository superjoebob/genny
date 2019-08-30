#include "UICheckBoxNum.h"
#include "GennyInterface.h"
#include "../resource.h"

UICheckBoxNum::UICheckBoxNum(const CRect& size, int num, CControlListener* listener, long tag, CBitmap* bitmap, GennyInterface* ins, bool special, const long style, int topoff):
	UICheckbox(size, listener, tag, "", ins, bitmap, style),
	_num(num),
	_special(special),
	_topOff(topoff)
{
	

}
	
UICheckBoxNum::~UICheckBoxNum()
{	
}
	
bool UICheckBoxNum::attached (CView* parent)
{
	bool returnValue = UICheckbox::attached(parent);
	
	if(_special) 
	{
		CRect pos = CRect( 0, 0, 12, 14 );
		if(size.width() > 20)
			pos.offset(size.left + 46, size.top + 2 + _topOff);
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

	return returnValue;
}
	
void UICheckBoxNum::setVisible(bool visible)
{
	_label->setVisible(visible);
	UICheckbox::setVisible(visible);
}
