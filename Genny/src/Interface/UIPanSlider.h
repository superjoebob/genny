#pragma once
#include "UIInstrument.h"
#include "UIImage.h"
#include "UICheckBoxNum.h"
#include "UIDigitDisplay.h"
#include "YM2612Enum.h"
#include "UISlider.h"
class UIPESelectedInstrument;
class UIPanSlider : public CControl, public CControlListener, public GennyInterfaceObject
{
public:
	UIPanSlider(const CPoint& vPosition, int vWidth, UIPESelectedInstrument* vOwner, bool vDelay = false);
	~UIPanSlider(void);
	virtual bool attached (CView* parent);	

	void setValue(float val);
	float getValue() const;
	virtual void valueChanged (CControl* control);
	void draw (CDrawContext* pContext);

	void reconnect();

	CLASS_METHODS(UIPanSlider, CControl)

private:
	UIPESelectedInstrument* _owner;
	UISlider* _slider;
	bool _delay;
	CTextLabel* _delayLabel;

	YM2612Param _param;
	int _op;
	float _width;
	CPoint _position;
};

