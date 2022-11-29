#pragma once
#include "UIInstrument.h"
#include "UIImage.h"
class UILetterDisplay : public CView
{
public:
	UILetterDisplay(const CPoint& pos, CView* owner, CFrame* frame, int letters, bool offMode = false);
	~UILetterDisplay(void);
	virtual void setText (const std::string& text);

	CLASS_METHODS(UILetterDisplay, CView)

private:
	CView* _owner;
	std::vector<UIImage*> _letters;
	std::string _text;
	bool _offMode;
};

