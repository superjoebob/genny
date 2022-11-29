#include "UILetterDisplay.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"

UILetterDisplay::UILetterDisplay(const CPoint& pos, CView* owner, CFrame* frame, int letters, bool offMode):
	CView(CRect()),
	_owner(owner),
	_offMode(offMode)
{
	CRect displaySize = CRect(0, 0, 32, 48);
	displaySize.offset(pos.x, pos.y);
	for(int i = 0; i < letters; i++)
	{ 
		UIImage* letter = new UIImage(displaySize, PNG_LETTERS, true);
		frame->addView(letter);
		displaySize.offset(32,0);
		_letters.push_back(letter);
	}
}

UILetterDisplay::~UILetterDisplay(void)
{

}

void UILetterDisplay::setText (const std::string& text)
{
	_text = text;
	

	int start = text.length() - 1;
	if(start >= _letters.size())
		start = _letters.size() - 1;

	for(int i = 0; i < _letters.size(); i++)
	{
		_letters[i]->setFrame(0);
	}


	int current = _letters.size() - 1;
	for(int i = start; i >= 0; i--)
	{
		char letter = text[i];
		if(letter > 64 && letter < 91)
			_letters[current]->setFrame((letter - 65) + 1);
		else if(letter > 96 && letter < 123)
			_letters[current]->setFrame((letter - 97) + 1);	
		else if(letter > 47 && letter < 58)
			_letters[current]->setFrame((letter - 48) + 27);
		else if(letter == 95)
			_letters[current]->setFrame(38);
		else
			_letters[current]->setFrame(0);
		current --;
	}

	setDirty(true);
} 
