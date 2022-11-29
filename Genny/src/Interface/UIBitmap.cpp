#include "UIBitmap.h"
UIBitmap::UIBitmap(int bitmap)
{
	_base = new CBitmap((CResourceDescription)bitmap);
}

UIBitmap::~UIBitmap(void)
{
	_base->forget();
}
