#include "UIInstrument.h"
#include "GennyInterface.h"
#include "GennyVST.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "UIFeedbackControl.h"
#include "UIAlgorithmSelector.h"
#include "UIChannelStatus.h"
#include "UIOperator.h"
#include "UIDigitKnob.h"
#include "UIDigitDisplay.h"
#include "UICheckbox.h"
#include "UIDigitSlider2.h"
#include "UIWaveForm.h";
#include "UISampleRangeSlider.h"

UIInstrument::UIInstrument(GennyInterface* owner):	
	GennyInterfaceObject(owner),
	_owner(owner),
	_drumLabel(nullptr),
	_lastDrumClick(0.0f)
{
}

UIInstrument::~UIInstrument(void)
{
}

const int kDrumButtonIndexStart = 459382;
const int kDrumButtonIndexEnd = 459382 + 20;

void UIInstrument::initialize()
{
	CFrame* frame = _owner->getFrame();
	IndexBaron* baron = _owner->getIndexBaron();

	CRect fbControlSize = CRect(0, 0, 0, 0);
	fbControlSize.offset(52, 186);
	UIFeedbackControl* fbControl = new UIFeedbackControl(fbControlSize , this);
	frame->addView(fbControl);
	_controls[fbControl->getTag()] = fbControl;
	
	CRect algControlSize = CRect(0, 0, 0, 0);
	algControlSize.offset(52, 186);
	UIAlgorithmSelector* algControl = new UIAlgorithmSelector(algControlSize , this);
	frame->addView(algControl);
	_controls[algControl->getTag()] = algControl;


	int index = getIndexBaron()->getYMParamIndex(YM_LFO_EN);
	UIBitmap specialButton(PNG_BIGLIGHT);
	CRect lfoSize = CRect(0, 0, 22, 22);
	lfoSize.offset(50, 258 - 158);
	UICheckbox* lfoEnCheckbox = new UICheckbox(lfoSize, this, index, "", _owner, specialButton);
	_globalControls[index] = lfoEnCheckbox;
	lfoEnCheckbox->setValue(getVst()->getPatch(0)->getFromBaron(baron->getIndex(index)));
	frame->addView(lfoEnCheckbox);


	index = baron->getYMParamIndex(YM_LFO);
	UIDigitKnob* lfoKnob = new UIDigitKnob(CPoint(116, 256 - 158), this, YM_LFO);
	_globalControls[index] = lfoKnob;
	lfoKnob->setMin(0);
	lfoKnob->setMax((float)YM2612Param_getRange(YM_LFO));
	lfoKnob->setValue(getPatch(0)->getFromBaron(baron->getIndex(index)));
	lfoKnob->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_LFO));

	index = baron->getYMParamIndex(YM_AMS);
	UIDigitKnob* amsKnob = new UIDigitKnob(CPoint(230, 256 - 158), this, YM_AMS);
	_controls[index] = amsKnob;
	amsKnob->setMin(0);
	amsKnob->setMax((float)YM2612Param_getRange(YM_AMS));
	amsKnob->setValue(getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	amsKnob->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_AMS));

	index = baron->getYMParamIndex(YM_FMS);
	UIDigitKnob* fmsKnob = new UIDigitKnob(CPoint(340, 256 - 158), this, YM_FMS);
	_controls[index] = fmsKnob;
	fmsKnob->setMin(0);
	fmsKnob->setMax((float)YM2612Param_getRange(YM_FMS));
	fmsKnob->setValue(getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	fmsKnob->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_FMS));

	_channelStatus = new UIChannelStatus(this);
	frame->addView(_channelStatus);
	
	int specialIndex = getIndexBaron()->getYMParamIndex(YM_SPECIAL);
	CRect specialSize = CRect(0, 0, 22, 22);
	specialSize.offset(372, 54);
	CCheckBox* specialMode = new CCheckBox(specialSize, this, specialIndex, "", specialButton);
	_globalControls[specialIndex] = specialMode;
	specialMode->setValue(getPatch(0)->getFromBaron(baron->getIndex(specialIndex)));
	frame->addView(specialMode);

	index = baron->getInsParamIndex(GIP_Octave);
	UIDigitKnob* octaveKnob = new UIDigitKnob(CPoint(870, 116), this, YM_NONE, GIP_Octave);
	octaveKnob->setMinMax(-3, 3);
	_controls[index] = octaveKnob;
	octaveKnob->setMin(0);
	octaveKnob->setMax((float)GennyInstrumentParam_getRange(GIP_Octave));
	octaveKnob->setValue(getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	octaveKnob->setWheelInc(1.0f / (float)GennyInstrumentParam_getRange(GIP_Octave));
	frame->addView(octaveKnob);
	
	for(int i = 0; i < 4; i++)
	{
		UIOperator* operatorControl = new UIOperator(this, i);
		_operators.push_back(operatorControl);
	}

	UIImage* image = new UIImage(CRect(44, 142, 44 + 356, 142 + 142), PNG_DOOR01, false);
	frame->addView(image);
	_doors.push_back(image);

	image = new UIImage(CRect(44, 94, 44 + 356, 94 + 30), PNG_DOOR02, false);
	frame->addView(image);
	_doors.push_back(image);

	image = new UIImage(CRect(250, 298, 250 + 222, 298 + 140), PNG_DOOR03, false);
	frame->addView(image);
	_doors.push_back(image);

	image = new UIImage(CRect(482, 298, 482 + 222, 298 + 140), PNG_DOOR03, false);
	frame->addView(image);
	_doors.push_back(image);

	image = new UIImage(CRect(714, 298, 714 + 222, 298 + 140), PNG_DOOR03, false);
	frame->addView(image);
	_doors.push_back(image);

	image = new UIImage(CRect(250, 442, 250 + 222, 442 + 92), PNG_DOOR04, false);
	frame->addView(image);
	_doors.push_back(image);

	image = new UIImage(CRect(250, 442, 250 + 222, 442 + 92), PNG_DOOR04, false);
	frame->addView(image);
	_doors.push_back(image);

	image = new UIImage(CRect(482, 442, 482 + 222, 442 + 92), PNG_DOOR04, false);
	frame->addView(image);
	_doors.push_back(image);

	image = new UIImage(CRect(714, 442, 714 + 222, 442 + 92), PNG_DOOR04, false);
	frame->addView(image);
	_doors.push_back(image);

	image = new UIImage(CRect(12, 436, 12 + 234, 436 + 98), PNG_DOOR05, false);
	frame->addView(image);
	_doors.push_back(image);
	_SNDoor = image;

	int drumyOffset = -144;
	image = new UIImage(CRect(12, 436 + drumyOffset, 12 + 234, 436 + 242 + drumyOffset), PNG_DOORDRUM, false);
	frame->addView(image);
	_doors.push_back(image);
	_drumDoor = image;

	index = baron->getYMParamIndex(SN_SR);
	UIDigitKnob* shiftKnob = new UIDigitKnob(CPoint(72, 468), this, SN_SR);
	_controls[index] = shiftKnob;
	shiftKnob->setMin(0);
	shiftKnob->setMax((float)YM2612Param_getRange(SN_SR));
	shiftKnob->setValue(getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	_snControls.push_back(shiftKnob);
	frame->addView(shiftKnob);
	shiftKnob->setWheelInc(1.0f / (float)YM2612Param_getRange(SN_SR));

	index = getIndexBaron()->getYMParamIndex(SN_PERIODIC);
	specialSize = CRect(0, 0, 22, 22);
	specialSize.offset(214, 470);

	UICheckbox* periodicMode = new UICheckbox(specialSize, this, index, "", _owner, specialButton);
	_controls[index] = periodicMode;
	periodicMode->setValue(getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	_snControls.push_back(periodicMode);
	frame->addView(periodicMode);




	
	index = getIndexBaron()->getYMParamIndex(YM_DRUMTL, 3);

	float change = 34;
	UIDigitSlider2* drumTLSlider = new UIDigitSlider2(CPoint(214 - (118 + change), 480 - 2), 104 + change, this, YM_DRUMTL, 3);
	_controls[index] = drumTLSlider;	
	_drumControls.push_back(drumTLSlider);
	frame->addView(drumTLSlider);

	drumTLSlider->setMin(0);
	drumTLSlider->setMax((float)YM2612Param_getRange(YM_DRUMTL));
	drumTLSlider->setValue(getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	drumTLSlider->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_DRUMTL)); 
	//drumTLSlider->getSlider()->centerStick = 100;


	index = getIndexBaron()->getYMParamIndex(SN_DT);
	UIDigitSlider2* dtSlider = new UIDigitSlider2(CPoint(214 - 118, 450 - 2), 104, this, SN_DT, -1);
	_controls[index] = dtSlider;	
	_snControls.push_back(dtSlider);
	frame->addView(dtSlider);

	dtSlider->setMin(0);
	dtSlider->setMax((float)YM2612Param_getRange(SN_DT));
	dtSlider->setValue(getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	dtSlider->setWheelInc(1.0f / (float)YM2612Param_getRange(SN_DT));





	int drumButtonsX = 12 + 20;
	int drumButtonsY = 436 + drumyOffset + 15;

	_drumLabel = new CTextLabel(CRect(drumButtonsX, drumButtonsY, drumButtonsX + 162, drumButtonsY + 18), "Click Note To Load Sample.");
	_drumLabel->setFont(kNormalFont);
	_drumLabel->setHoriAlign(kLeftText);
	_drumLabel->getFont()->setStyle(kBoldFace);
	_drumLabel->setFontColor(CColor(16, 20, 16, 255));
	frame->addView(_drumLabel);
	_drumLabel->setMouseableArea(CRect());
	_drumLabel->setBackColor(CColor(0, 0, 0, 0));
	_drumLabel->setFrameColor(CColor(0, 0, 0, 0));

	drumButtonsX += 0;
	drumButtonsY += 63;

	int selectedDrum = ((GennyPatch*)getCurrentPatch())->InstrumentDef.SelectedDrum;
	for(int xpos = 0; xpos < 4; xpos++)
	{
		for(int ypos = 0; ypos < 5; ypos++)
		{
			int bX = drumButtonsX + (xpos * (42 + 8));
			int bY = drumButtonsY + (ypos * (14 + 2));

			int drumIndex = (xpos * 5) + (ypos);
			WaveData* drum = ((GennyPatch*)getCurrentPatch())->InstrumentDef.Drumset.getDrum(36 + drumIndex);

			CKickButton* drumButton = new CKickButton(CRect(bX + 4, bY, bX + 4 + 40, bY + 14), this, kDrumButtonIndexStart + ((xpos * 5) + (ypos)), 14, UIBitmap(IDB_PNG18), CPoint(xpos * 40, (drum != nullptr ? (drumIndex == selectedDrum ? 280 : 0) : 140) + (ypos * 28)));

			UIImage* drumLight = new UIImage(CRect(bX, bY, bX + 2, bY + 14), IDB_PNG19, false);
			drumLight->setFrame(4);

			_drumButtons.push_back(drumButton);
			_drumLights.push_back(drumLight);
			_drumLightVals.push_back(0.0f);

			frame->addView(drumButton);
			frame->addView(drumLight);
		}
	}

	_wave = new UIWaveForm(CPoint(drumButtonsX , drumButtonsY - 46), this);
	frame->addView(_wave);

	_sampleRangeLow = new UISampleRangeSlider(CPoint(drumButtonsX , drumButtonsY - 10), 156, this, true);
	frame->addView(_sampleRangeLow);

	_sampleRangeHigh = new UISampleRangeSlider(CPoint(drumButtonsX , drumButtonsY - 10), 156, this, false);
	frame->addView(_sampleRangeHigh);

	rangeTop = drumButtonsY - 10;
	rangeBottom = (drumButtonsY - 10) + 10;


	WaveData* selDrum = ((GennyPatch*)getCurrentPatch())->InstrumentDef.Drumset.getDrum(36 + selectedDrum);

	int sButtonX = drumButtonsX + 168;
	int sButtonY =  drumButtonsY - 46;
	UICheckbox* sampleL = new UICheckbox(CRect(sButtonX, sButtonY, sButtonX + 12, sButtonY + 14), this, 0, "", _owner, new CBitmap(IDB_PNG24));
	_controls[index] = sampleL;

	if(selDrum != nullptr)
		sampleL->setValue(selDrum->panLeft);

	_drumControls.push_back(sampleL);
	frame->addView(sampleL);


	sButtonX += 14;
	UICheckbox* sampleR = new UICheckbox(CRect(sButtonX, sButtonY, sButtonX + 12, sButtonY + 14), this, 0, "", _owner, new CBitmap(IDB_PNG25));
	_controls[index] = sampleR;

	if(selDrum != nullptr)
		sampleR->setValue(selDrum->panRight);

	_drumControls.push_back(sampleR);
	frame->addView(sampleR);

	sButtonX -= 14;
	sButtonY += 18;
	UICheckbox* sampleRepeat = new UICheckbox(CRect(sButtonX, sButtonY, sButtonX + 12, sButtonY + 14), this, 0, "", _owner, new CBitmap(IDB_PNG26));
	_controls[index] = sampleRepeat;
	_sampleLoop = sampleRepeat;

	if(selDrum != nullptr)
		sampleRepeat->setValue(selDrum->loop);

	_drumControls.push_back(sampleRepeat);
	frame->addView(sampleRepeat);

	sButtonX += 14;
	UICheckbox* sampleNormalize = new UICheckbox(CRect(sButtonX, sButtonY, sButtonX + 12, sButtonY + 14), this, 0, "", _owner, new CBitmap(IDB_PNG27));
	_controls[index] = sampleNormalize;
	_sampleNormalize = sampleNormalize;

	if(selDrum != nullptr)
		sampleNormalize->setValue(selDrum->normalize);

	_drumControls.push_back(sampleNormalize);
	frame->addView(sampleNormalize);






	reconnect();
}

void UIInstrument::mapControl(CControl* control, int index) 
{ 
	_controls[index] = control; 
}

void UIInstrument::setParam(int index, float val)
{
	std::map<int, CControl*>::iterator it = _controls.find(index);
	if(it != _controls.end())
	{
		_controls[index]->setValue(val);
		//if(index == _pitchIndex)
		//	refreshPitchDigits();
		
	}
	else
	{
		std::map<int, CControl*>::iterator it = _globalControls.find(index);
		if(it != _globalControls.end())
		{
			_globalControls[index]->setValue(val);
			//if(index == _pitchIndex)
			//	refreshPitchDigits();

		}
	}
}

void UIInstrument::makeOperatorDirty(int op) 
{ 
	_operators[op]->setDirty(true); 
}

void UIInstrument::valueChanged (CControl* control)
{	
	if(control == _sampleRangeLow)
	{
		if(control->getValue() > _sampleRangeHigh->getValue())
			control->setValue(_sampleRangeHigh->getValue());

		_wave->invalid();
		return;
	}
	else if(control == _sampleRangeHigh)
	{
		if(control->getValue() < _sampleRangeLow->getValue())
			control->setValue(_sampleRangeLow->getValue());

		_wave->invalid();
		return;
	}
	else if(control == _sampleLoop)
	{
		WaveData* wave = ((GennyPatch*)getCurrentPatch())->InstrumentDef.Drumset.getDrum(36 + ((GennyPatch*)getCurrentPatch())->InstrumentDef.SelectedDrum);
		if(wave != nullptr)
			wave->loop = _sampleLoop->getValue() > 0.3f;
		return;
	}
	else if(control == _sampleNormalize)
	{
		WaveData* wave = ((GennyPatch*)getCurrentPatch())->InstrumentDef.Drumset.getDrum(36 + ((GennyPatch*)getCurrentPatch())->InstrumentDef.SelectedDrum);
		if(wave != nullptr)
			wave->normalize = _sampleLoop->getValue() > 0.3f;
		return;
	}


	//if(control->getTag() == _pitchIndex)
	//	refreshPitchDigits();

	if(control->getTag() >= kDrumButtonIndexStart && control->getTag() <= kDrumButtonIndexEnd)
	{
		int drumIndex = control->getTag() - kDrumButtonIndexStart;
		WaveData* drum = ((GennyPatch*)getCurrentPatch())->InstrumentDef.Drumset.getDrum(36 + drumIndex);

		if(drum == nullptr || _lastDrumClick > 0.0f)
		{	
			if(control->getValue() == 1.0f)
				_owner->openImportSample(control->getTag() - kDrumButtonIndexStart);

			drum = ((GennyPatch*)getCurrentPatch())->InstrumentDef.Drumset.getDrum(36 + drumIndex);
		}

		if(drum != nullptr)
		{
			if(control->getValue() == 1.0f)
				_lastDrumClick = 1.0f;

			((GennyPatch*)getCurrentPatch())->InstrumentDef.SelectedDrum = control->getTag() - kDrumButtonIndexStart;
			reconnect();
		}
	}
	else
		_owner->valueChanged(control);
}

void UIInstrument::reconnect()
{
	IndexBaron* baron = getIndexBaron();

	int index = baron->getInsParamIndex(GIP_FM);
	int isFM = (int)(getCurrentPatch()->InstrumentDef.Type == GIType::FM || getCurrentPatch()->InstrumentDef.Type == GIType::DAC);
	int isDrums = (int)(getCurrentPatch()->InstrumentDef.Type == GIType::DAC);
	for(int i = 0; i < _doors.size(); i++)
	{
		if(_doors[i] == _drumDoor)
			_doors[i]->setVisible((isFM > 0 && isDrums > 0) ? true : false);
		else if(_doors[i] == _SNDoor)
			_doors[i]->setVisible(isFM > 0 ? false : true);
		else
		{
			_doors[i]->setVisible((isFM <= 0 || (isDrums > 0)) ? true : false);
		}

		_doors[i]->invalid();
	}

	std::map<int, CControl*>::iterator it;
	for(it = _controls.begin(); it != _controls.end(); it++)
	{
		(*it).second->setValue(getVst()->getCurrentPatch()->getFromBaron(baron->getIndex((*it).second->getTag())));
	}
	for(it = _globalControls.begin(); it != _globalControls.end(); it++)
	{
		(*it).second->setValue(getVst()->getPatch(0)->getFromBaron(baron->getIndex((*it).second->getTag())));
	}

	for(size_t i = 0; i < _refreshList.size(); i++)
	{
		_refreshList[i]->setDirty(true);
	}
	_channelStatus->reconnect();




	for(int i = 0; i < _drumControls.size(); i++)
	{
		_drumControls[i]->setVisible((isFM > 0 && isDrums > 0) ? true : false);
		_drumControls[i]->invalid();
	}

	for(int i = 0; i < _snControls.size(); i++)
	{
		_snControls[i]->setVisible((isFM <= 0 && isDrums == false) ? true : false);
		_snControls[i]->invalid();
	}

	if(_drumDoor->isVisible())
	{
		_wave->setVisible(true);
		_wave->setDirty(true);
		_drumLabel->setVisible(true);
		_sampleRangeHigh->setVisible(true);
		_sampleRangeLow->setVisible(true);
	}	
	else
	{
		_wave->setVisible(false);
		_wave->setDirty(true);
		_drumLabel->setVisible(false);
		_sampleRangeHigh->setVisible(false);
		_sampleRangeLow->setVisible(false);
	}


	//WaveData* wave = ((GennyPatch*)getCurrentPatch())->InstrumentDef.Drumset.getDrum(36 + ((GennyPatch*)getCurrentPatch())->InstrumentDef.SelectedDrum);
	//if(wave != nullptr)
	//{
	//	int low = wave->startSample;
	//	int high = wave->endSample;

	//	int middleSize = (((high - low) / (float)wave->size) * 158) / 2;
	//	int lowLeft = (12 + 20);
	//	int lowRight =  (12 + 20) + (((low / (float)wave->size) * 158) + middleSize);
	//	int highLeft = lowRight;
	//	int highRight = (12 + 20) + 158;

	//

	//	//_sampleRangeLow->setMin(0);
	//	//_sampleRangeLow->setMax(high);
	//	//_sampleRangeLow->sma(CRect(lowLeft, rangeTop, lowRight, rangeBottom));
	//	//_sampleRangeLow->invalid();

	//	////_sampleRangeHigh->setMin(low);
	//	////_sampleRangeHigh->setMax(wave->size);
	//	//_sampleRangeHigh->sma(CRect(highLeft, rangeTop, highRight, rangeBottom));
	//	//_sampleRangeHigh->invalid();
	//}


	WaveData* wave = ((GennyPatch*)getCurrentPatch())->InstrumentDef.Drumset.getDrum(36 + ((GennyPatch*)getCurrentPatch())->InstrumentDef.SelectedDrum);
	if(wave != nullptr)
	{
		_sampleLoop->setValue(wave->loop ? 1.0f : 0.0f);
		_sampleNormalize->setValue(wave->normalize ? 1.0f : 0.0f);
	}

	_sampleRangeHigh->reconnect();
	_sampleRangeLow->reconnect();

	int selectedDrum = ((GennyPatch*)getCurrentPatch())->InstrumentDef.SelectedDrum;
	int drumButtonCount = _drumButtons.size();
	for(int i = 0; i < drumButtonCount; i++)
	{
		if(_drumDoor->isVisible())
		{	
			_drumButtons[i]->setVisible(true);
			_drumLights[i]->setVisible(true);

			int drumIndex = _drumButtons[i]->getTag() - kDrumButtonIndexStart;
			WaveData* drum = ((GennyPatch*)getCurrentPatch())->InstrumentDef.Drumset.getDrum(36 + drumIndex);

			int buttonX = (_drumButtons[i]->getTag() - kDrumButtonIndexStart) / 5;
			int buttonY = (_drumButtons[i]->getTag() - kDrumButtonIndexStart) % 4;
			_drumButtons[i]->setBackground(UIBitmap(IDB_PNG18));
			_drumButtons[i]->setBackgroundOffsetNotBroken(CPoint(buttonX * 40, (drum != nullptr ? (drumIndex == selectedDrum ? 280 : 0) : 140) + (buttonY * 28)));
			_drumButtons[i]->setVisible(false);
			_drumButtons[i]->setVisible(true);
			_drumButtons[i]->invalid();
			_drumButtons[i]->setDirty(true);

			if(drum != nullptr && drumIndex == selectedDrum)
			{
				_drumLabel->setText(drum->sampleName.c_str());
				_drumLabel->invalid();
			}
		}
		else
		{
			_drumButtons[i]->setVisible(false);
			_drumLights[i]->setVisible(false);
		}
	}

	if(_drumDoor->isVisible())
	{
		_wave->setVisible(false);
		_wave->setVisible(true);
	}
	_wave->setDirty(true);
	_wave->invalid();

	_operators[0]->reconnect(_drumDoor->isVisible() || isFM <= 0);
	_operators[1]->reconnect(_drumDoor->isVisible() || isFM <= 0);
	_operators[2]->reconnect(_drumDoor->isVisible() || isFM <= 0);
	_operators[3]->reconnect(_drumDoor->isVisible() || isFM <= 0);
	makeChannelsDirty();

}

void UIInstrument::makeChannelsDirty()
{
	_channelStatus->makeDirty();
}

void UIInstrument::updateChannels()
{
	_channelStatus->update();

	if(_lastDrumClick > 0)
		_lastDrumClick -= 0.1f;
	else
		_lastDrumClick = 0;

	int drums = _drumLights.size();
	for(int i = 0; i < drums; i++)
	{
		WaveData* drum = ((GennyPatch*)getCurrentPatch())->InstrumentDef.Drumset.getDrum(36 + i);

		if(drum != nullptr && drum->flash)
		{
			_drumLightVals[i] = 1.0f;
			drum->flash = false;
		}
		else
		{
			if(_drumLightVals[i] > 0.0f)
				_drumLightVals[i] -= 0.15f;
			else
				_drumLightVals[i] = 0.0f;
		}

		_drumLights[i]->setFrame((1.0f - _drumLightVals[i]) * 4);
	}
}
