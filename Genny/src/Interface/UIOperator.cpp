#include "UIOperator.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIDigitDisplay.h"
#include "UIDigitSlider.h"
#include "UIReverseKnob.h"
#include "UIOpKnob.h"
#include "UIKnob.h"
#include "UICheckbox.h"
#include "UIMidiChannel.h"
#include "UISpinner.h"
#include "UIPanSlider.h"
UIOperator::UIOperator(UIInstrument* owner, int number):
	CControl(CRect(64 + (number * 232), 330, 64 + (number * 232) + 164, 330 + 62 + 14), owner),
	GennyInterfaceObject(owner),
	_owner(owner),
	_operator(number),
	_opSpDisplay(nullptr),
	_detuneSlider(nullptr)
{
	int offset = number * 232;
	int yOffset = 372;


	if (_operator == 1)
		_operator = 2;
	else if (_operator == 2)
		_operator = 1;

	CFrame* frame = getInterface()->getFrame();
	IndexBaron* baron = getIndexBaron();

	UIBitmap knobImage(PNG_KNOB);

	setMouseableArea(CRect());
	setMouseEnabled(false);


	_owner->addRefreshable(this);

	float lowerKnobYOffset = 66;
	float lowerSliderYOffset = 30;

	int index = baron->getYMParamIndex(YM_TL, _operator);
	CRect knobSize = CRect(0, 0, 26, 26);
	knobSize.offset(26 + offset, yOffset);
	UIOpKnob* levelKnob = new UIOpKnob(knobSize, this, index, 31, 26.0, knobImage, getInterface());
	levelKnob->directParent = this;
	frame->addView(levelKnob);
	_owner->mapControl(levelKnob, index);
	levelKnob->setMin(0);
	levelKnob->setMax((float)YM2612Param_getRange(YM_TL));
	levelKnob->setValue(getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	levelKnob->setWheelInc((1.0f / (float)YM2612Param_getRange(YM_TL)));

	index = baron->getYMParamIndex(YM_AR, _operator);
	knobSize = CRect(0 , 0, 26, 26);

	int lowerEnvKnobXOff = 32;
	knobSize.offset(lowerEnvKnobXOff + offset, yOffset + lowerKnobYOffset);
	UIOpKnob* attackKnob = new UIOpKnob(knobSize, this, index, 31, 26.0, knobImage, getInterface());
	attackKnob->directParent = this;
	frame->addView(attackKnob);
	_owner->mapControl(attackKnob, index);
	attackKnob->setMin(0);
	attackKnob->setMax((float)YM2612Param_getRange(YM_AR));
	attackKnob->setValue(getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	attackKnob->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_AR));

	lowerEnvKnobXOff += 42;
	index = baron->getYMParamIndex(YM_DR, _operator);
	knobSize = CRect(0 , 0, 26, 26);
	knobSize.offset(lowerEnvKnobXOff + offset, yOffset + lowerKnobYOffset);
	UIOpKnob* decay1Knob = new UIOpKnob(knobSize, this, index, 31, 26.0, knobImage, getInterface());
	decay1Knob->directParent = this;
	frame->addView(decay1Knob);
	_owner->mapControl(decay1Knob, index);
	decay1Knob->setMin(0);
	decay1Knob->setMax((float)YM2612Param_getRange(YM_DR));
	decay1Knob->setValue(getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	decay1Knob->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_DR));

	lowerEnvKnobXOff += 42;
	index = baron->getYMParamIndex(YM_SL, _operator);
	knobSize = CRect(0 , 0, 26, 26);
	knobSize.offset(lowerEnvKnobXOff + offset, yOffset + lowerKnobYOffset);
	UIOpKnob* sustainKnob = new UIOpKnob(knobSize, this, index, 31, 26.0, knobImage, getInterface());
	sustainKnob->directParent = this;
	frame->addView(sustainKnob);
	_owner->mapControl(sustainKnob, index);
	sustainKnob->setMin(0);
	sustainKnob->setMax((float)YM2612Param_getRange(YM_SL));
	sustainKnob->setValue(getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	sustainKnob->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_SL));

	lowerEnvKnobXOff += 42;
	index = baron->getYMParamIndex(YM_SR, _operator);
	knobSize = CRect(0 , 0, 26, 26);
	knobSize.offset(lowerEnvKnobXOff + offset, yOffset + lowerKnobYOffset);
	UIOpKnob* decay2Knob = new UIOpKnob(knobSize, this, index, 31, 26.0, knobImage, getInterface());
	decay2Knob->directParent = this;
	frame->addView(decay2Knob);
	_owner->mapControl(decay2Knob, index);
	decay2Knob->setMin(0);
	decay2Knob->setMax((float)YM2612Param_getRange(YM_SR));
	decay2Knob->setValue(getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	decay2Knob->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_SR));

	lowerEnvKnobXOff += 42;
	index = baron->getYMParamIndex(YM_RR, _operator);
	knobSize = CRect(0 , 0, 26, 26);
	knobSize.offset(lowerEnvKnobXOff + offset, yOffset + lowerKnobYOffset);
	UIOpKnob* releaseKnob = new UIOpKnob(knobSize, this, index, 31, 26.0, knobImage, getInterface());
	releaseKnob->directParent = this;
	frame->addView(releaseKnob);
	_owner->mapControl(releaseKnob, index);
	releaseKnob->setMin(0);
	releaseKnob->setMax((float)YM2612Param_getRange(YM_RR));
	releaseKnob->setValue(getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	releaseKnob->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_RR));





	int lowerSliderPos = 466;
	int lowerSliderXOff = 0;
	index = baron->getYMParamIndex(YM_DT, _operator);
	CRect sliderSize = CRect();
	UIDigitSlider* detuneSlider = new UIDigitSlider(CPoint(92 + lowerSliderXOff, lowerSliderPos + lowerSliderYOffset), 110, this, YM_DT, _operator, offset);
	frame->addView(detuneSlider);
	_owner->mapControl(detuneSlider, index);
	detuneSlider->setMin(0);
	detuneSlider->setMax((float)YM2612Param_getRange(YM_DT) - 1);
	detuneSlider->setValue(getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	detuneSlider->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_DT));

	lowerSliderYOffset += 22;
	index = baron->getYMParamIndex(YM_MUL, _operator);
	sliderSize = CRect();
	UIDigitSlider* freqSlider = new UIDigitSlider(CPoint(72 + lowerSliderXOff, lowerSliderPos + lowerSliderYOffset), 130, this, YM_MUL, _operator, offset);
	frame->addView(freqSlider);
	_owner->mapControl(freqSlider, index);
	freqSlider->setMin(0);
	freqSlider->setMax((float)YM2612Param_getRange(YM_MUL));
	freqSlider->setValue(getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	freqSlider->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_MUL));

	lowerSliderYOffset += 22;
	index = baron->getYMParamIndex(YM_KS, _operator);
	sliderSize = CRect();
	UIDigitSlider* envSlider = new UIDigitSlider(CPoint(124 + lowerSliderXOff, lowerSliderPos + lowerSliderYOffset), 78, this, YM_KS, _operator, offset);
	frame->addView(envSlider);
	_owner->mapControl(envSlider, index);
	envSlider->setMin(0);
	envSlider->setMax((float)YM2612Param_getRange(YM_KS));
	envSlider->setValue(getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	envSlider->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_KS));

	lowerSliderYOffset += 22;
	index = baron->getYMParamIndex(YM_SSG, _operator);
	sliderSize = CRect();
	UIDigitSlider* ssgSlider = new UIDigitSlider(CPoint(124 + lowerSliderXOff, lowerSliderPos + lowerSliderYOffset), 78, this, YM_SSG, _operator, offset);
	frame->addView(ssgSlider);
	_owner->mapControl(ssgSlider, index);
	ssgSlider->setMin(0);
	ssgSlider->setMax((float)YM2612Param_getRange(YM_SSG));
	ssgSlider->setValue(getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	ssgSlider->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_SSG));


	index = getIndexBaron()->getYMParamIndex(YM_AM, _operator);
	UIBitmap specialButton(PNG_BIGLIGHT);
	CRect lfoSize = CRect(0, 0, 22, 22);
	lfoSize.offset(56 + offset + lowerSliderXOff, (lowerSliderPos + lowerSliderYOffset) - 2);
	UICheckbox* lfoEnCheckbox = new UICheckbox(lfoSize, this, index, "", getInterface(), specialButton);
	_owner->mapControl(lfoEnCheckbox, index);
	lfoEnCheckbox->setValue(getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	lfoEnCheckbox->setMin(0);
	lfoEnCheckbox->setMax((float)YM2612Param_getRange(YM_AM));
	frame->addView(lfoEnCheckbox);

	lfoSize = CRect(0, 0, 22, 22);
	lfoSize.offset(56 + offset + lowerSliderXOff, (lowerSliderPos + lowerSliderYOffset) - 24);
	_operatorVelocity = new UICheckbox(lfoSize, this, index, "", getInterface(), specialButton);
	_operatorVelocity->setValue(getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	_operatorVelocity->setMin(0);
	_operatorVelocity->setMax((float)YM2612Param_getRange(YM_AM));
	_operatorVelocity->setExtParam(getCurrentPatch()->getExt(GEParam::OpVelocity, _operator));
	frame->addView(_operatorVelocity);





	UIBitmap opImage(IDB_PNG31 + number);
	CRect opSize = CRect(0, 0, 32, 28); 
	opSize.offset(18 + offset, 318);
	_operatorEnable = new UICheckbox(opSize, this, kOperatorVelocityStart + _operator, "", getInterface(), opImage);

	bool en = getCurrentPatch()->InstrumentDef.OperatorEnable[_operator];
	_operatorEnable->setValue(en);
	_operatorEnable->setMin(0);
	_operatorEnable->setMax(1);
	frame->addView(_operatorEnable);


	CRect opSpPos = CRect(0, 0, 64, 76);
	opSpPos.offset(offset + 66, yOffset - 42);
	_opSpDisplay = new UIImage(opSpPos, PNG_OPSPSETTINGS);
	_opSpDisplay->setMouseEnabled(false);
	_opSpDisplay->setMouseableArea(CRect());
	frame->addView(_opSpDisplay);
	_opSpDisplay->setVisible(false);

	_opChSel = new UIMidiChannel(CPoint(offset + 68, yOffset - 26), this, this, _operator);
	frame->addView(_opChSel);
	_opChSel->setVisible(false);

	_opOctave = new UISpinner(CPoint(offset + 68, yOffset - 2), this, GennyInstrumentParam::GIP_Ch3OpOctave, _operator);
	frame->addView(_opOctave);
	_opOctave->setVisible(false);	
	
	_opTranspose = new UISpinner(CPoint(offset + 68, yOffset + 18), this, GennyInstrumentParam::GIP_Ch3OpTranspose, _operator);
	frame->addView(_opTranspose);
	_opTranspose->setVisible(false);


	CPoint detunePos = CPoint(offset + 116, yOffset - 30);
	_detuneSlider = new UIPanSlider(detunePos, 62, this, this, UIPanSliderType::DetuneVert, _operator);
	frame->addView(_detuneSlider);


	reconnect(false);
	setDirty(true);
	invalid();
}

UIOperator::~UIOperator(void)
{

}

void UIOperator::reconnect(bool drumMode)
{
	bool ch3 = getCurrentPatch()->InstrumentDef.Ch3Special;
	_opSpDisplay->setVisible(ch3);
	_opOctave->setVisible(ch3);
	_opOctave->reconnect();
	_opTranspose->setVisible(ch3);
	_opTranspose->reconnect();
	_opChSel->reconnect(); 


	//GennyExtParam* p = getCurrentPatch()->getExt(GEParam::Op3SpecialDetune, _operator);
	_detuneSlider->setVisible(ch3);
	_detuneSlider->reconnect();
	//_detuneSlider->setValue();


	_operatorEnable->setVisible(!drumMode);

	if (!drumMode)
	{
		_operatorEnable->setExtParam(getCurrentPatch()->getExt(GEParam::OpEnable, _operator)); 
		_operatorVelocity->setExtParam(getCurrentPatch()->getExt(GEParam::OpVelocity, _operator));
	}

	setDirty(true);
	invalid();
}


void UIOperator::setValue(float val)
{ 
	CControl::setValue(val);
}

void UIOperator::valueChanged (CControl* control)
{
	if (control == _detuneSlider)
	{
		GennyExtParam* p = getCurrentPatch()->getExt(GEParam::Op3SpecialDetune, _operator);

		//p->set()

		return;
	}


	//if (control == _operatorVelocity)
	//{
	//	getCurrentPatch()->InstrumentDef.OperatorVelocity[_operator] = control->getValue() > 0.5f ? true : false;
	//}
	//else
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

	CPoint drawOffset = CPoint(0, 20);

	int offset = (offsetNumber * 232);

	IndexBaron* baron = _owner->getIndexBaron(); 
	float tl = (getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(baron->getYMParamIndex(YM_TL, _operator))) / (float)YM2612Param_getRange(YM_TL));
	float ar = 1.0f - getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(baron->getYMParamIndex(YM_AR, _operator))) / (float)YM2612Param_getRange(YM_AR);
	float d1r = (getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(baron->getYMParamIndex(YM_DR, _operator))) / (float)YM2612Param_getRange(YM_DR));
	float sus = 1.0f - getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(baron->getYMParamIndex(YM_SL, _operator))) / (float)YM2612Param_getRange(YM_SL);
	float d2r = getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(baron->getYMParamIndex(YM_SR, _operator))) / (float)YM2612Param_getRange(YM_SR);
	float rr = getVst()->getCurrentPatch()->getFromBaron(baron->getIndex(baron->getYMParamIndex(YM_RR, _operator))) / (float)YM2612Param_getRange(YM_RR);

	if (((GennyPatch*)getVst()->getCurrentPatch())->InstrumentDef.Type == GIType::SN || ((GennyPatch*)getVst()->getCurrentPatch())->InstrumentDef.Type == GIType::SNDRUM || ((GennyPatch*)getVst()->getCurrentPatch())->InstrumentDef.Type == GIType::SNSPECIAL)
	{
		rr = 1.0f - rr;
		d1r = 1.0f - d1r;
	}


	rr = rr * rr;



	float width = 162;
	if (getCurrentPatch()->InstrumentDef.Ch3Special)
	{
		width -= 64;
		drawOffset.x += 64;
	}

	float height = 60 + 14;
	int topper = 371 + 14;

	pContext->setFrameColor(CColor(15, 19, 15, 255));
	pContext->setLineStyle(kLineSolid);
	pContext->setLineWidth(2);

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


	CPoint pos = CPoint((66 + offset) + arOffset, topper - tlOffset);
	CPoint oldPos = pos;

	pContext->drawLine(CPoint(66 + offset, topper) + drawOffset, pos + drawOffset); oldPos = pos;
	pos.offset(drOffset, susOffset);
	pContext->drawLine(oldPos + drawOffset, pos + drawOffset); oldPos = pos;
	pos.offset(extraOffset, dr2Offset);
	pContext->drawLine(oldPos + drawOffset, pos + drawOffset); oldPos = pos;
	pos.offset(rrOffset, (tlOffset - susOffset - dr2Offset));
	pContext->drawLine(oldPos + drawOffset, pos + drawOffset); oldPos = pos;

	pContext->setFrameColor(CColor(39, 55, 31, 255));
	CPoint movePoint = CPoint((66 + offset) + arOffset, topper - tlOffset);
	pContext->drawLine(movePoint + drawOffset, CPoint(movePoint.x, topper) + drawOffset);
	movePoint.offset(drOffset, susOffset);
	pContext->drawLine(movePoint + drawOffset, CPoint(movePoint.x, topper) + drawOffset);
	movePoint.offset(extraOffset, dr2Offset);
	pContext->drawLine(movePoint + drawOffset, CPoint(movePoint.x, topper) + drawOffset);

	CControl::setDirty(false);
	//pContext->lineTo(CPoint(0, 0));
	//pContext->lineTo(CPoint(100, 100));
}