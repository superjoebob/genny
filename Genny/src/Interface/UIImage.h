#pragma once
#include "UIInstrument.h"
#include "UIBitmap.h"
class UIImage : public CView
{
public:
	UIImage(const CRect& size, int image, bool lcd = false);
	~UIImage(void);

	void setFrame(int frame) { if (frame != _frame) { _frame = frame; setDirty(true); invalid(); } }
	int getNumFrames() { return (int)(_bitmap.getHeight() / CView::getHeight()); }

	virtual void setDirty(bool dirty);

	virtual void draw (CDrawContext* pContext);

	CLASS_METHODS(UIImage, CView)

		int _frame;

private:
	float _width;
	float _height;
	int _prevFrame;
	UIBitmap _bitmap;
	UIBitmap _bitmap2;
	bool _oneMore;
	bool _doOne;
	bool _lcd;
};

