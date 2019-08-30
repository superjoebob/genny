#pragma once
#include "UIInstrument.h"
#include "UIImage.h"
#include "UICheckBoxNum.h"
#include "UIDigitDisplay.h"
#include "YM2612Enum.h"
#include "UISlider.h"
class UIPESelectedInstrument;
class UIRangeSlider : public CControl, public CControlListener, public GennyInterfaceObject
{
public:
	UIRangeSlider(const CPoint& vPosition, int vWidth, UIPESelectedInstrument* vOwner, bool vLow);
	~UIRangeSlider(void);
	virtual bool attached (CView* parent);	

	void setValue(float val);
	float getValue() const;
	virtual void valueChanged (CControl* control);
	void draw (CDrawContext* pContext);

	void reconnect();

	CLASS_METHODS(UIRangeSlider, CControl)

private:
	UIPESelectedInstrument* _owner;
	UISlider* _slider;

	bool _low;
	YM2612Param _param;
	int _op;
	float _width;
	CPoint _position;
};

