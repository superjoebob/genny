#pragma once
#include "UIInstrument.h"
#include "UIImage.h"
#include "UICheckBoxNum.h"
#include "UIDigitDisplay.h"
#include "YM2612Enum.h"
#include "UISlider.h"

enum UIPanSliderType
{
	Pan,
	Delay,
	Glide,
	DetuneHor,
	DetuneVert,
	SamplePitch
};

class UIPanSlider : public CControl, public IControlListener, public GennyInterfaceObject
{
public:
	UIPanSlider(const CPoint& vPosition, int vWidth, GennyInterfaceObject* vOwner, IControlListener* pListener, UIPanSliderType eType, int op = 0);
	~UIPanSlider(void);
	virtual bool attached (CView* parent);	

	void setValue(float val);
	float getValue() const;
	virtual void valueChanged (CControl* control);
	void draw (CDrawContext* pContext);
	virtual void setVisible(bool visible);
	void updateLabel();
	 
	void reconnect();

	UISlider* _slider;
	CLASS_METHODS(UIPanSlider, CControl)

private:
	UIPanSliderType _type;
	CTextLabel* _delayLabel;

	YM2612Param _param;
	int _op;
	float _width;
	CPoint _position;
};

