#include "UIBitmap.h"
UIBitmap::UIBitmap(int bitmap)
{
	_base = new CBitmap(bitmap);
}

UIBitmap::~UIBitmap(void)
{
	_base->forget();
}
