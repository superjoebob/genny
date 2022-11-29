#include "UIDigitDisplay.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"

UIDigitDisplay::UIDigitDisplay(const CPoint& pos, CFrame* frame, int digits, bool offMode):
	CView(CRect()),
	_offMode(offMode)
{

	if (offMode)
	{
		CRect displaySize = CRect(0, 0, 32, 18);
		displaySize.offset(pos.x - 2, pos.y - 2);
		UIImage* digit = new UIImage(displaySize, PNG_SSGDISP, true);
		frame->addView(digit);
		_digits.push_back(digit);
	}
	else
	{
		CRect displaySize = CRect(0, 0, 8, 14);
		displaySize.offset(pos.x, pos.y);
		for (int i = 0; i < digits; i++)
		{
			UIImage* digit = new UIImage(displaySize, PNG_DIGITS, true);
			frame->addView(digit);
			displaySize.offset(10, 0);
			_digits.push_back(digit);
		}
	}
}

UIDigitDisplay::~UIDigitDisplay(void)
{

}

void UIDigitDisplay::setNumber (int val)
{
	_number = val;
	if(val < 0)
		val *= -1;

	if (_offMode)
	{
		_digits[0]->setFrame(val);
		return;
	}

	if(_offMode && val == 0 && _digits.size() == 3)
	{
		_digits[0]->setFrame(1);
		_digits[1]->setFrame(11);
		_digits[2]->setFrame(11);
		return;
	}

	char buf[6];
	itoa(val, buf, 10);
	int len = strlen(buf);

	for(int i = 0; i < _digits.size(); i++)
	{
		_digits[i]->setFrame(0);
	}

	if(len > _digits.size())
		return;

	if(_number < 0)
	{
		if(len == 2)
		{
			_digits[_digits.size() - 3]->setFrame(12);
			_digits[_digits.size() - 2]->setFrame((buf[0] - 48) + 1);
			_digits[_digits.size() - 1]->setFrame((buf[1] - 48) + 1);
		}
		if(len == 1)
		{
			_digits[_digits.size() - 2]->setFrame(12);
			_digits[_digits.size() - 1]->setFrame((buf[0] - 48) + 1);
		}
	}
	else
	{
		if (len == 5)
		{
			_digits[_digits.size() - 5]->setFrame((buf[0] - 48) + 1);
			_digits[_digits.size() - 4]->setFrame((buf[1] - 48) + 1);
			_digits[_digits.size() - 3]->setFrame((buf[2] - 48) + 1);
			_digits[_digits.size() - 2]->setFrame((buf[3] - 48) + 1);
			_digits[_digits.size() - 1]->setFrame((buf[4] - 48) + 1);
		}
		if (len == 4)
		{
			_digits[_digits.size() - 4]->setFrame((buf[0] - 48) + 1);
			_digits[_digits.size() - 3]->setFrame((buf[1] - 48) + 1);
			_digits[_digits.size() - 2]->setFrame((buf[2] - 48) + 1);
			_digits[_digits.size() - 1]->setFrame((buf[3] - 48) + 1);
		}
		if(len == 3)
		{
			_digits[_digits.size() - 3]->setFrame((buf[0] - 48) + 1);
			_digits[_digits.size() - 2]->setFrame((buf[1] - 48) + 1);
			_digits[_digits.size() - 1]->setFrame((buf[2] - 48) + 1);
		}
		if(len == 2)
		{
			_digits[_digits.size() - 2]->setFrame((buf[0] - 48) + 1);
			_digits[_digits.size() - 1]->setFrame((buf[1] - 48) + 1);
		}
		if(len == 1)
		{
			_digits[_digits.size() - 1]->setFrame((buf[0] - 48) + 1);
		}
	}
	setDirty(true);
} 

void UIDigitDisplay::invalid()
{
	for(int i = 0; i < _digits.size(); i++)
	{
		_digits[i]->invalid();
	}
	CView::invalid();
}

void UIDigitDisplay::setVisible(bool visible)
{
	for(int i = 0; i < _digits.size(); i++)
	{
		_digits[i]->setVisible(visible);
	}
}
