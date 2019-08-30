#include "UIContextMenu.h"

UIContextMenu::UIContextMenu(float xpos, float ypos):
	CView(CRect(xpos, ypos, xpos + 100, ypos + 100)),
	_frame(0),
	_prevFrame(0),
	_oneMore(false),
	_doOne(false),
	_x(xpos),
	_y(ypos)
{

}

UIContextMenu::~UIContextMenu(void)
{

}

void UIContextMenu::setDirty(bool dirty)
{
	__super::setDirty(dirty);
	if(dirty)
		_doOne = true;
}

void UIContextMenu::draw (CDrawContext* pContext)
{
	pContext->setFillColor(CColor(30, 30, 30, 255));
	pContext->drawRect(CRect(_x, _y, _x + 100, _y + 100), kDrawFilled);
	CView::setDirty(false);
}