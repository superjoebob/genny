#pragma once
#include "UIInstrument.h"
#include "UIImage.h"
#include "UICheckBoxNum.h"
#include "UIDigitDisplay.h"
#include "YM2612Enum.h"
#include "UISlider.h"
class UIInstrument;
class UISampleRangeSlider : public CControl, public CControlListener, public GennyInterfaceObject
{
public:
	UISampleRangeSlider(const CPoint& pos, int width, UIInstrument* owner, bool low);
	~UISampleRangeSlider(void);
	void setValue(float val);
	float getValue() const;
	virtual void valueChanged (CControl* control);
	void draw (CDrawContext* pContext);

	virtual void setMin (float val) { _slider->setMin(val); }
	virtual void setMax (float val) { _slider->setMax(val); }

	void svs(const CRect& r);
	void sma(const CRect& r);

	void reconnect();	
	void setVisible(bool visible);

	CLASS_METHODS(UISampleRangeSlider, CControl)

private:
	UIInstrument* _owner;
	UISlider* _slider;
	CPoint _pos;
	float _wide;

	bool _low;
	YM2612Param _param;
	int _op;
};

