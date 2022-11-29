#pragma once
#include "UIImage.h"
#include "UICheckbox.h"
class GennyInterface;
class UIAlgorithmSelector;
class UICheckBoxNum : public UICheckbox
{
public:
	UICheckBoxNum(const CRect& size, int num, IControlListener* listener, long tag, CBitmap* bitmap, GennyInterface* ins, bool special = false, const long style = (1 << 4), int topoff = 0, UIAlgorithmSelector* algSelector = nullptr);
	~UICheckBoxNum();
	virtual CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons);

	virtual CMouseEventResult onMouseEntered(CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseExited(CPoint& where, const CButtonState& buttons);
	virtual bool onWheel(const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons);

	virtual bool attached (CView* parent);	
	virtual void setVisible(bool visible);

	void SetNum( int num );
	int GetNum() { return _num; }

	CLASS_METHODS(UICheckBoxNum, UICheckbox)
private:
	int _num;
	int _topOff;
	bool _special;
	UIAlgorithmSelector* _algSelector;
	UIImage* _label;
};
