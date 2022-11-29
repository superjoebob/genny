#pragma once
#include "UIInstrument.h"
#include "UIImage.h"
#include "UICheckBoxNum.h"
#include "UIDigitDisplay.h"
#include "YM2612Enum.h"
#include "UISlider.h"

class UIPESelectedInstrument;
class UIRangeSlider : public CControl, public IControlListener, public GennyInterfaceObject
{
public:
	UIRangeSlider(const CPoint& vPosition, int vWidth, GennyInterfaceObject* vOwner, IControlListener* listener, bool vLow, bool flipRight = false);
	~UIRangeSlider(void);
	virtual bool attached (CView* parent);	
	virtual void setVisible(bool visible);

	void setValue(float val);
	float getValue() const;
	virtual void valueChanged (CControl* control);
	void draw (CDrawContext* pContext);

	void reconnect();
	UISlider* _slider;

	CLASS_METHODS(UIRangeSlider, CControl)

private:
	bool _low;
	bool _flipRight;
	YM2612Param _param;
	int _op;
	float _width;
	CPoint _position;
};

