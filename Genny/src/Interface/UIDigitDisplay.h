#pragma once
#include "UIInstrument.h"
#include "UIImage.h"
class UIDigitDisplay : public CView
{
public:
	UIDigitDisplay(const CPoint& pos, CFrame* owner, int digits, bool offMode = false);
	~UIDigitDisplay(void);
	virtual void setNumber (int val);
	virtual void invalid();
	virtual void setVisible(bool visible);

	CLASS_METHODS(UIDigitDisplay, CView)

private:
	std::vector<UIImage*> _digits;
	int _number;
	bool _offMode;
};

