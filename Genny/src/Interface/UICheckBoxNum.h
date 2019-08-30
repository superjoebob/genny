#pragma once
#include "UIImage.h"
#include "UICheckbox.h"
class GennyInterface;
class UICheckBoxNum : public UICheckbox
{
public:
	UICheckBoxNum(const CRect& size, int num, CControlListener* listener, long tag, CBitmap* bitmap, GennyInterface* ins, bool special = false, const long style = kRight, int topoff = 0);
	~UICheckBoxNum();
	virtual bool attached (CView* parent);	
	virtual void setVisible(bool visible);

	void SetNum( int num );
	int GetNum() { return _num; }

	CLASS_METHODS(UICheckBoxNum, UICheckbox)
private:
	int _num;
	int _topOff;
	bool _special;
	UIImage* _label;
};
