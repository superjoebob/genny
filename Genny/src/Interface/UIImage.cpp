#include "UIImage.h"

UIImage::UIImage(const CRect& size, int image, bool lcd):
	CView(size),
	_bitmap(image),
	_bitmap2(image),
	_frame(0),
	_prevFrame(0),
	_oneMore(false),
	_lcd(lcd),
	_doOne(false)
{

}

UIImage::~UIImage(void)
{

}

void UIImage::setDirty(bool dirty)
{
	__super::setDirty(dirty);
	if(dirty)
		_doOne = true;
}

void UIImage::draw (CDrawContext* pContext)
{
	setDirty(false);
	if(_prevFrame != _frame && _lcd)
	{
		if(_oneMore == false)
		{
			CBitmap* map1 = _bitmap.getBase();
			map1->draw(pContext, size, CPoint(0, size.height() * _prevFrame), 0.7f);
			CBitmap* map2 = _bitmap2.getBase();
			map2->draw(pContext, size, CPoint(0, size.height() * _frame), 0.3f);

			_oneMore = true;
			setDirty(true);
		}
		else
		{
			CBitmap* map1 = _bitmap.getBase();
			map1->draw(pContext, size, CPoint(0, size.height() * _prevFrame), 0.3f);
			CBitmap* map2 = _bitmap2.getBase();
			map2->draw(pContext, size, CPoint(0, size.height() * _frame), 0.7f);

			setDirty(true);
			_oneMore = false;
			_prevFrame = _frame;
		}
	}
	else
	{
		CBitmap* map = _bitmap.getBase();
		map->draw(pContext, size, CPoint(0, size.height() * _frame));
		setDirty(false);
		if(_doOne)
		{
			invalid();
			_doOne = false;
		}
	}
}