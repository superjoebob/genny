#include "UIDigitSlider.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIOperator.h"
#include "UIDigitDisplay.h"

int dtFunTable[8]={0,1,2,3,0,-1,-2,-3};
int dtReverseTable[8]={7, 6, 5, 0, 1, 2, 3};

UIDigitSlider::UIDigitSlider(const CPoint& pos, int width, UIOperator* owner, YM2612Param param, int op, int offset):
	CControl(CRect(), owner),
	GennyInterfaceObject(owner),
	_owner(owner),
	_param(param),
	_op(op)
{
	CFrame* frame = getInterface()->getFrame();
	IndexBaron* baron = owner->getIndexBaron();
	int index = baron->getYMParamIndex(param, op);
	tag = index;

	UIBitmap sliderKnob(PNG_SLIDERKNOB);

	CRect sliderSize = CRect(0, 0, width , 28);
	sliderSize.offset(pos.x + offset, pos.y);
	_slider = new UISlider(sliderSize, this, index, pos.x + offset, pos.x + offset + (width - 16), sliderKnob, NULL, getInterface());
	frame->addView(_slider);
	_slider->setBackground(NULL);



	_display = new UIDigitDisplay(CPoint(pos.x + offset + width + 4, pos.y + 2), owner->getFrame(), 3, param == YM_SSG);
	frame->addView(_display);


	float value = getInterface()->getCurrentPatch()->getFromBaron(baron->getIndex(index));
	setValue(value);
}

UIDigitSlider::~UIDigitSlider(void)
{

}

void UIDigitSlider::setValue(float val)
{
	float realVal = val;
	if(_param == YM_DT)
	{
		int funNum = (int)(value + 0.5f);
		if (funNum >= 0 && funNum < 8)
		{
			int real = dtFunTable[funNum] + 3;
			realVal = real;
		}
	}


	_slider->setValue(realVal);


	updateDisplay();
} 

float UIDigitSlider::getValue() const
{
	float realVal = _slider->getValue();
	if(_param == YM_DT)
	{
		int val = (int)(realVal + 0.5f);
		int real = dtReverseTable[val];
		realVal = (float)real;
	}
	return realVal;
}

void UIDigitSlider::valueChanged (CControl* control)
{
	updateDisplay();
	_owner->valueChanged(this);
}

void UIDigitSlider::draw (CDrawContext* pContext)
{
	CControl::setDirty(false);
}

void UIDigitSlider::updateDisplay()
{
	float realVal = _slider->getValue();
	if(_param == YM_DT)
	{
		int real = (int)(realVal + 0.5f);
		real -= 3;
		_display->setNumber(real);
	}
	else
		_display->setNumber((int)(realVal + 0.5f));
}