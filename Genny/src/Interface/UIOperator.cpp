#include "UIOperator.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIDigitDisplay.h"
#include "UIDigitSlider.h"
#include "UIReverseKnob.h"
#include "UIKnob.h"
#include "UICheckbox.h"
UIOperator::UIOperator(UIInstrument* owner, int number):
	CControl(CRect(66 + (number * 232), 310, 66 + (number * 232) + 162, 310 + 62), owner),
	GennyInterfaceObject(owner),
	_owner(owner),
	_operator(number)
{
	int offset = number * 232;

	if (_operator == 1)
		_operator = 2;
	else if (_operator == 2)
		_operator = 1;

	CFrame* frame = owner->getFrame();
	frame->addView(this);

	IndexBaron* baron = getIndexBaron();

	UIBitmap knobImage(PNG_KNOB);


	_owner->addRefreshable(this);

	int index = baron->getYMParamIndex(YM_TL, _operator);
	CRect knobSize = CRect(0, 0, 26, 26);
	knobSize.offset(26 + offset, 352);
	UIReverseKnob* levelKnob = new UIReverseKnob(knobSize, this, index, 31, 26.0, knobImage, getInterface());
	levelKnob->directParent = this;
	frame->addView(levelKnob);
	_owner->mapControl(levelKnob, index);
	levelKnob->setMin(0);
	levelKnob->setMax((float)YM2612Param_getRange(YM_TL));
	levelKnob->setValue(getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	levelKnob->setWheelInc(-(1.0f / (float)YM2612Param_getRange(YM_TL)));

	index = baron->getYMParamIndex(YM_AR, _operator);
	knobSize = CRect(0 , 0, 26, 26);
	knobSize.offset(32 + offset, 404);
	UIKnob* attackKnob = new UIKnob(knobSize, this, index, 31, 26.0, knobImage, getInterface());
	attackKnob->directParent = this;
	frame->addView(attackKnob);
	_owner->mapControl(attackKnob, index);
	attackKnob->setMin(0);
	attackKnob->setMax((float)YM2612Param_getRange(YM_AR));
	attackKnob->setValue(getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	attackKnob->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_AR));


	index = baron->getYMParamIndex(YM_DR, _operator);
	knobSize = CRect(0 , 0, 26, 26);
	knobSize.offset(74 + offset, 404);
	UIKnob* decay1Knob = new UIKnob(knobSize, this, index, 31, 26.0, knobImage, getInterface());
	decay1Knob->directParent = this;
	frame->addView(decay1Knob);
	_owner->mapControl(decay1Knob, index);
	decay1Knob->setMin(0);
	decay1Knob->setMax((float)YM2612Param_getRange(YM_DR));
	decay1Knob->setValue(getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	decay1Knob->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_DR));


	index = baron->getYMParamIndex(YM_SL, _operator);
	knobSize = CRect(0 , 0, 26, 26);
	knobSize.offset(116 + offset, 404);
	UIKnob* sustainKnob = new UIKnob(knobSize, this, index, 31, 26.0, knobImage, getInterface());
	sustainKnob->directParent = this;
	frame->addView(sustainKnob);
	_owner->mapControl(sustainKnob, index);
	sustainKnob->setMin(0);
	sustainKnob->setMax((float)YM2612Param_getRange(YM_SL));
	sustainKnob->setValue(getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	sustainKnob->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_SL));


	index = baron->getYMParamIndex(YM_SR, _operator);
	knobSize = CRect(0 , 0, 26, 26);
	knobSize.offset(158 + offset, 404);
	UIKnob* decay2Knob = new UIKnob(knobSize, this, index, 31, 26.0, knobImage, getInterface());
	decay2Knob->directParent = this;
	frame->addView(decay2Knob);
	_owner->mapControl(decay2Knob, index);
	decay2Knob->setMin(0);
	decay2Knob->setMax((float)YM2612Param_getRange(YM_SR));
	decay2Knob->setValue(getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	decay2Knob->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_SR));


	index = baron->getYMParamIndex(YM_RR, _operator);
	knobSize = CRect(0 , 0, 26, 26);
	knobSize.offset(200 + offset, 404);
	UIKnob* releaseKnob = new UIKnob(knobSize, this, index, 31, 26.0, knobImage, getInterface());
	releaseKnob->directParent = this;
	frame->addView(releaseKnob);
	_owner->mapControl(releaseKnob, index);
	releaseKnob->setMin(0);
	releaseKnob->setMax((float)YM2612Param_getRange(YM_RR));
	releaseKnob->setValue(getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	releaseKnob->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_RR));


	index = baron->getYMParamIndex(YM_DT, _operator);
	CRect sliderSize = CRect();
	UIDigitSlider* detuneSlider = new UIDigitSlider(CPoint(92, 446), 110, this, YM_DT, _operator);
	frame->addView(detuneSlider);
	_owner->mapControl(detuneSlider, index);
	detuneSlider->setMin(0);
	detuneSlider->setMax((float)YM2612Param_getRange(YM_DT) - 1);
	detuneSlider->setValue(getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	detuneSlider->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_DT));


	index = baron->getYMParamIndex(YM_MUL, _operator);
	sliderSize = CRect();
	UIDigitSlider* freqSlider = new UIDigitSlider(CPoint(72, 468), 130, this, YM_MUL, _operator);
	frame->addView(freqSlider);
	_owner->mapControl(freqSlider, index);
	freqSlider->setMin(0);
	freqSlider->setMax((float)YM2612Param_getRange(YM_MUL));
	freqSlider->setValue(getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	freqSlider->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_MUL));


	index = baron->getYMParamIndex(YM_KS, _operator);
	sliderSize = CRect();
	UIDigitSlider* envSlider = new UIDigitSlider(CPoint(124, 490), 78, this, YM_KS, _operator);
	frame->addView(envSlider);
	_owner->mapControl(envSlider, index);
	envSlider->setMin(0);
	envSlider->setMax((float)YM2612Param_getRange(YM_KS));
	envSlider->setValue(getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	envSlider->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_KS));


	index = baron->getYMParamIndex(YM_SSG, _operator);
	sliderSize = CRect();
	UIDigitSlider* ssgSlider = new UIDigitSlider(CPoint(124, 512), 78, this, YM_SSG, _operator);
	frame->addView(ssgSlider);
	_owner->mapControl(ssgSlider, index);
	ssgSlider->setMin(0);
	ssgSlider->setMax((float)YM2612Param_getRange(YM_SSG));
	ssgSlider->setValue(getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	ssgSlider->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_SSG));


	index = getIndexBaron()->getYMParamIndex(YM_AM, _operator);
	UIBitmap specialButton(PNG_BIGLIGHT);
	CRect lfoSize = CRect(0, 0, 22, 22);
	lfoSize.offset(56 + offset, 509);
	UICheckbox* lfoEnCheckbox = new UICheckbox(lfoSize, this, index, "", getInterface(), specialButton);
	_owner->mapControl(lfoEnCheckbox, index);
	lfoEnCheckbox->setValue(getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	lfoEnCheckbox->setMin(0);
	lfoEnCheckbox->setMax((float)YM2612Param_getRange(YM_AM));
	frame->addView(lfoEnCheckbox);






	UIBitmap opImage(IDB_PNG31 + number);
	CRect opSize = CRect(0, 0, 32, 28); 
	opSize.offset(18 + offset, 298);
	_operatorVelocity = new UICheckbox(opSize, this, 0, "", getInterface(), opImage);

	bool en = getCurrentPatch()->InstrumentDef.OperatorVelocity[_operator];
	_operatorVelocity->setValue(en);
	_operatorVelocity->setMin(0);
	_operatorVelocity->setMax(1);
	frame->addView(_operatorVelocity);

	setDirty(true);
}

UIOperator::~UIOperator(void)
{

}

void UIOperator::reconnect(bool drumMode)
{
	if (drumMode)
		_operatorVelocity->setVisible(false);
	else
	{
		_operatorVelocity->setVisible(true);
		bool en = getCurrentPatch()->InstrumentDef.OperatorVelocity[_operator];
		_operatorVelocity->setValue(en);
	}
}


void UIOperator::setValue(float val)
{
	CControl::setValue(val);
}

void UIOperator::valueChanged (CControl* control)
{
	if (control == _operatorVelocity)
	{
		getCurrentPatch()->InstrumentDef.OperatorVelocity[_operator] = control->getValue() > 0.5f ? true : false;
	}
	else
	{
		setDirty(true);
		_owner->valueChanged(control);
	}
}

void UIOperator::draw (CDrawContext* pContext)
{
	int offsetNumber = _operator;
	if (offsetNumber == 2)
		offsetNumber = 1;
	else if (offsetNumber == 1)
		offsetNumber = 2;

	int offset = offsetNumber * 232;

	IndexBaron* baron = _owner->getIndexBaron();
	float tl = 1.0f - (getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(baron->getYMParamIndex(YM_TL, _operator))) / (float)YM2612Param_getRange(YM_TL));
	float ar = getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(baron->getYMParamIndex(YM_AR, _operator))) / (float)YM2612Param_getRange(YM_AR);
	float d1r = 1.0f - (getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(baron->getYMParamIndex(YM_DR, _operator))) / (float)YM2612Param_getRange(YM_DR));
	float sus = getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(baron->getYMParamIndex(YM_SL, _operator))) / (float)YM2612Param_getRange(YM_SL);
	float d2r = getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(baron->getYMParamIndex(YM_SR, _operator))) / (float)YM2612Param_getRange(YM_SR);
	float rr = 1.0f - getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(baron->getYMParamIndex(YM_RR, _operator))) / (float)YM2612Param_getRange(YM_RR);
	rr = rr * rr;


	float width = 162;
	float height = 60;

	pContext->setFrameColor(CColor(15, 19, 15, 255));
	pContext->setLineStyle (kLineSolid);
	pContext->setLineWidth(2);
	pContext->moveTo(CPoint(66 + offset, 371));

	float arOffset = (1.0f - ar) * (width / 4);
	float tlOffset = tl * height;
	float susOffset = sus * (tlOffset);
	float drOffset = d1r * (width / 4);
	float dr2Offset = d2r * (tlOffset - susOffset);
	float rrOffset = rr * (width / 4);

	float extraOffset = width / 4;

	float totalWidth = arOffset + drOffset + extraOffset + rrOffset;
	float mul = width / totalWidth;


	arOffset *= mul;
	drOffset *= mul;
	rrOffset *= mul;
	extraOffset *= mul;


	CPoint pos = CPoint((66 + offset) + arOffset, 371 - tlOffset);

	pContext->lineTo(pos);
	pos.offset(drOffset, susOffset);
	pContext->lineTo(pos);
	pos.offset(extraOffset, dr2Offset);
	pContext->lineTo(pos);
	pos.offset(rrOffset, (tlOffset - susOffset - dr2Offset));
	pContext->lineTo(pos);

	pContext->setFrameColor(CColor(39, 55, 31, 255));
	CPoint movePoint = CPoint((66 + offset) + arOffset, 371 - tlOffset);
	pContext->moveTo(movePoint);
	pContext->lineTo(CPoint(movePoint.x, 371));
	movePoint.offset(drOffset, susOffset);
	pContext->moveTo(movePoint);
	pContext->lineTo(CPoint(movePoint.x, 371));
	movePoint.offset(extraOffset, dr2Offset);
	pContext->moveTo(movePoint);
	pContext->lineTo(CPoint(movePoint.x, 371));

	CControl::setDirty(false);
	//pContext->lineTo(CPoint(0, 0));
	//pContext->lineTo(CPoint(100, 100));
}