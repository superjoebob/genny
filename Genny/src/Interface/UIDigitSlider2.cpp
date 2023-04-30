#include "UIDigitSlider2.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIOperator.h"
#include "UIDigitDisplay.h"

int dtFunTable2[8]={0,1,2,3,0,-1,-2,-3};
int dtReverseTable2[8]={7, 6, 5, 0, 1, 2, 3};

UIDigitSlider2::UIDigitSlider2(const CPoint& pos, int width, UIInstrument* owner, YM2612Param param, int op):
	CControl(CRect(), owner),
	GennyInterfaceObject(owner),
	_owner(owner),
	_param(param),
	_op(op)
{
	int offset = 0;

	CFrame* frame = getInterface()->getFrame();
	IndexBaron* baron = getIndexBaron();
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


	float value = getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(index));
	setValue(value);
}

UIDigitSlider2::~UIDigitSlider2(void)
{

}

void UIDigitSlider2::setValue(float val)
{
	float realVal = val;
	if(_param == YM_DT)
	{
		int real = dtFunTable2[(int)(val + 0.5f)] + 3;
		realVal = real;
	}


	_slider->setValue(realVal);


	updateDisplay();
} 

float UIDigitSlider2::getValue() const
{
	float realVal = _slider->getValue();
	if(_param == YM_DT)
	{
		int val = (int)(realVal + 0.5f);
		int real = dtReverseTable2[val];
		realVal = (float)real;
	}
	return realVal;
}

void UIDigitSlider2::valueChanged (CControl* control)
{
	updateDisplay();
	_owner->valueChanged(this);
}

void UIDigitSlider2::draw (CDrawContext* pContext)
{
	CControl::setDirty(false);
}

void UIDigitSlider2::updateDisplay()
{
	float realVal = _slider->getValue();
	if(_param == SN_DT)
	{
		int real = (int)(realVal + 0.5f);
		real -= 50;
		_display->setNumber(real);
	}
	else
		_display->setNumber((int)(realVal + 0.5f));
}

void UIDigitSlider2::setVisible(bool visible)
{
	if(_slider != NULL)
		_slider->setVisible(visible);

	if(_display != NULL)
		_display->setVisible(visible);

	CControl::setVisible(visible);
}
