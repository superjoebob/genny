#pragma once

#include "GennyInterface.h"
class UIBitmap
{
public:
	UIBitmap(int bitmap);
	~UIBitmap(void);

	double getWidth() { return _base->getWidth(); }
	double getHeight() { return _base->getHeight(); }
	operator CBitmap*() { return _base; }
	CBitmap* getBase() { return _base; }

private:
	CBitmap* _base;
};

