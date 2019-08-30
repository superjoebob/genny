#pragma once
#include "UIInstrument.h"
#include "UIBitmap.h"
class UIContextMenu : public CView
{
public:
	UIContextMenu(float xpos, float ypos);
	~UIContextMenu(void);

	virtual void setDirty(bool dirty);
	void setPosition(float xpos, float ypos)
	{ 
		_x = xpos; 
		_y = ypos; 

		CRect size = getViewSize();
		size.moveTo(_x, _y);
		setViewSize(size, true);

		this->invalid(); 
	}
	virtual void draw (CDrawContext* pContext);

	CLASS_METHODS(UIContextMenu, CView)

private:
	float _width;
	float _height;
	int _frame;
	int _prevFrame;
	bool _oneMore;
	bool _doOne;
	bool _lcd;

	float _x;
	float _y;
};

