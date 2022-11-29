#include "UIDigitKnob.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIOperator.h"
#include "UIDigitDisplay.h"

UIDigitKnob::UIDigitKnob(const CPoint& pos, UIInstrument* owner, YM2612Param param, GennyInstrumentParam param2, bool hzKnob):
	CControl(CRect(), owner),
	GennyInterfaceObject(owner),
	_owner(owner),
	_param(param),
	_display(nullptr),
	_knob(nullptr),
	_YMParam(true),
	_insParam(param2),
	_min(0),
	_max(0),
	_hzKnob(hzKnob)
{
	CFrame* frame = getInterface()->getFrame();
	IndexBaron* baron = getIndexBaron();

	int index = 0;
	if(param != YM_NONE)
		index = baron->getYMParamIndex(param);
	else
	{
		index = baron->getInsParamIndex(param2);
		_YMParam = false;
	}
	tag = index;

	UIBitmap knobImage(PNG_KNOB);
	CRect knobSize = CRect(0, 0, 26, 26);
	knobSize.offset(pos.x, pos.y);
	_knob = new UIKnob(knobSize, this, index, 31, 26.0, knobImage, getInterface());

	frame->addView(_knob);

	if(_hzKnob)
		_display = new UIDigitDisplay(CPoint(pos.x + 32, pos.y + 6), frame, 5);
	else
		_display = new UIDigitDisplay(CPoint(pos.x + 32, pos.y + 6), frame, 2);
	frame->addView(_display);

	if(index >= 0)
		setValue(getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(index)));
}

UIDigitKnob::~UIDigitKnob(void)
{

}

void UIDigitKnob::setValue(float val)
{
	CControl::setValue(val);
	_knob->setValue(val);


	float realVal = (int)(val + 0.5f);
	if(_insParam == GIP_Octave)
	{
		realVal = realVal - 3;
	}

	if (getExtParam() != nullptr && getExtParam()->param == GEParam::DACSamplerate)
	{
		int rl = (int)(val + 0.5f);
		if (rl < 0)
			rl = 0;
		if (rl > 3)
			rl = 3;

		realVal = kDACSamplerates[rl];
	}


	_display->setNumber(realVal);
}

void UIDigitKnob::valueChanged (CControl* control)
{
	setValue(control->getValue());
	_owner->valueChanged(this);
}

void UIDigitKnob::draw (CDrawContext* pContext)
{
	CControl::setDirty(false);
}

void UIDigitKnob::invalid()
{
	if(_display != nullptr)
	{
		_display->invalid();
		_knob->invalid();
	}
	CControl::invalid();
}
void UIDigitKnob::setVisible(bool visible)
{
	if(_display != nullptr)
	{
		_display->setVisible(visible);
		_knob->setVisible(visible);
	}
}