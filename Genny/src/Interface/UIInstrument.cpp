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
#include "UIRangeSlider.h"
#include "UIPanSlider.h"

UIInstrument::UIInstrument(GennyInterface* owner):	
	GennyInterfaceObject(owner),
	_owner(owner),
	_drumLabel(nullptr),
	_lastDrumClick(0.0f),
	_specialMode(nullptr),
	_lastDrumClickTag(0)
{
}

UIInstrument::~UIInstrument(void)
{
}

const int kDrumButtonIndexStart = 459382;
const int kDrumButtonIndexEnd = 459382 + 20;
const int kTrueStereoIndex = 666666;

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

	//Global LFO
	//---------------------------------------------------------
	UIBitmap specialButton(PNG_BIGLIGHT);
	UIBitmap specialButtonInverse(IDB_PNG42);	
	CRect lfoSize = CRect(0, 0, 22, 22);
	lfoSize.offset(272, 98);

	/*int index = getIndexBaron()->getYMParamIndex(YM_LFO_EN);
	UICheckbox* lfoEnCheckbox = new UICheckbox(lfoSize, this, index, "", _owner, specialButton);
	_globalControls[index] = lfoEnCheckbox;
	lfoEnCheckbox->setValue(getVst()->getPatch(0)->getFromBaron(baron->getIndex(index)));
	frame->addView(lfoEnCheckbox);*/



	int index = baron->getYMParamIndex(YM_LFO);
	_lfoKnob = new UIDigitKnob(CPoint(338, 94), this, YM_LFO);
	_globalControls[index] = _lfoKnob;
	_lfoKnob->setMin(0);
	_lfoKnob->setMax((float)YM2612Param_getRange(YM_LFO));
	_lfoKnob->setValue(getPatch(0)->getFromBaron(baron->getIndex(index)));
	_lfoKnob->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_LFO));
	frame->addView(_lfoKnob);
	//---------------------------------------------------------


	//Channel LFO
	//---------------------------------------------------------
	lfoSize = CRect(0, 0, 22, 22);
	lfoSize.offset(CPoint(56, 276));
	_lfoEnCheckbox = new UICheckbox(lfoSize, this, index, "", _owner, specialButton);
	_lfoEnCheckbox->setExtParam(getCurrentPatch()->getExt(GEParam::LFOEnable));
	frame->addView(_lfoEnCheckbox);


	index = baron->getYMParamIndex(YM_AMS);
	UIDigitKnob* amsKnob = new UIDigitKnob(CPoint(210, 274), this, YM_AMS);
	_controls[index] = amsKnob;
	amsKnob->setMin(0);
	amsKnob->setMax((float)YM2612Param_getRange(YM_AMS));
	amsKnob->setValue(getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	amsKnob->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_AMS));
	frame->addView(amsKnob);

	index = baron->getYMParamIndex(YM_FMS);
	UIDigitKnob* fmsKnob = new UIDigitKnob(CPoint(330, 274), this, YM_FMS);
	_controls[index] = fmsKnob;
	fmsKnob->setMin(0);
	fmsKnob->setMax((float)YM2612Param_getRange(YM_FMS));
	fmsKnob->setValue(getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	fmsKnob->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_FMS));
	frame->addView(fmsKnob);
	//---------------------------------------------------------


	lfoSize = CRect(0, 0, 22, 22);
	lfoSize.offset(56, 100);
	_ch3SpecialBox = new UICheckbox(lfoSize, this, index, "", _owner, specialButton);
	_ch3SpecialBox->setExtParam(getCurrentPatch()->getExt(GEParam::Op3Special));
	frame->addView(_ch3SpecialBox);




	_channelStatus = new UIChannelStatus(this);
	frame->addView(_channelStatus);
	
	CRect specialSize = CRect(0, 0, 22, 22);
	specialSize.offset(374, 52);
	_specialMode = new CCheckBox(specialSize, this, kTrueStereoIndex, "", specialButton);
	_specialMode->setValue(baron->enableTrueStereo ? 1.0f : 0.0f);
	frame->addView(_specialMode);

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
		frame->addView(operatorControl);
	}


	int doorYOff = 20;
	UIImage* image = new UIImage(CRect(44, 122 - 30, 44 + 356, (122 - 30) + 210), PNG_DOOR01, false);
	frame->addView(image);
	_doors.push_back(image);

	//image = new UIImage(CRect(44, 94, 44 + 126, 94 + 30), PNG_DOOR02, false);
	//frame->addView(image);
	//_doors.push_back(image);

	image = new UIImage(CRect(250, 298 + doorYOff, 250 + 222, 298 + doorYOff + 170), PNG_DOOR03, false);
	frame->addView(image);
	_doors.push_back(image);

	image = new UIImage(CRect(482, 298 + doorYOff, 482 + 222, 298 + doorYOff + 170), PNG_DOOR03, false);
	frame->addView(image);
	_doors.push_back(image);

	image = new UIImage(CRect(714, 298 + doorYOff, 714 + 222, 298 + doorYOff + 170), PNG_DOOR03, false);
	frame->addView(image);
	_doors.push_back(image);

	image = new UIImage(CRect(250, 442 + 30 + doorYOff, 250 + 222, 442 + doorYOff + 92 + 28), PNG_DOOR04, false);
	frame->addView(image);
	_doors.push_back(image);
	 
	image = new UIImage(CRect(250, 442 + doorYOff + 30, 250 + 222, 442 + doorYOff + 92 + 30), PNG_DOOR04, false);
	frame->addView(image);
	_doors.push_back(image);

	image = new UIImage(CRect(482, 442 + doorYOff + 30, 482 + 222, 442 + doorYOff + 92 + 30), PNG_DOOR04, false);
	frame->addView(image);
	_doors.push_back(image);

	image = new UIImage(CRect(714, 442 + doorYOff + 30, 714 + 222, 442 + doorYOff + 92 + 30), PNG_DOOR04, false);
	frame->addView(image);
	_doors.push_back(image);

	image = new UIImage(CRect(12, 436 + 34 + doorYOff, 12 + 234, 436 + doorYOff + 98 + 34), PNG_DOOR05, false);
	frame->addView(image);
	_doors.push_back(image);
	_SNDoor = image;


	int drumyOffset = -124;
	CPoint drumsPos = CPoint(12, 436 + drumyOffset);
	image = new UIImage(CRect(drumsPos.x, drumsPos.y, drumsPos.x + 234, drumsPos.y + 278), PNG_DOORDRUM, false);
	frame->addView(image);
	_doors.push_back(image);
	_drumDoor = image;

	index = baron->getYMParamIndex(SN_SR);
	UIDigitKnob* shiftKnob = new UIDigitKnob(CPoint(72, 468 + 34 + doorYOff), this, SN_SR);
	_controls[index] = shiftKnob;
	shiftKnob->setMin(0);
	shiftKnob->setMax((float)YM2612Param_getRange(SN_SR));
	shiftKnob->setValue(getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	_snControls.push_back(shiftKnob);
	shiftKnob->setWheelInc(1.0f / (float)YM2612Param_getRange(SN_SR));
	frame->addView(shiftKnob);

	index = getIndexBaron()->getYMParamIndex(SN_PERIODIC);
	specialSize = CRect(0, 0, 22, 22);
	specialSize.offset(214, 470 + 34 + doorYOff);

	UICheckbox* periodicMode = new UICheckbox(specialSize, this, index, "", _owner, specialButtonInverse);
	_controls[index] = periodicMode;
	periodicMode->setValue(getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	_snControls.push_back(periodicMode);
	frame->addView(periodicMode);




	
	index = getIndexBaron()->getYMParamIndex(YM_DRUMTL, 3);

	float change = 34;
	UIDigitSlider2* drumTLSlider = new UIDigitSlider2(CPoint(212 - (118 + change), 480 + 54), 86 + change + doorYOff, this, YM_DRUMTL, 3);
	_controls[index] = drumTLSlider;	
	_drumControls.push_back(drumTLSlider);
	drumTLSlider->setMin(0);
	drumTLSlider->setMax((float)YM2612Param_getRange(YM_DRUMTL));
	drumTLSlider->setValue(getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	drumTLSlider->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_DRUMTL));
	frame->addView(drumTLSlider);


	index = getIndexBaron()->getYMParamIndex(SN_DT);
	UIDigitSlider2* dtSlider = new UIDigitSlider2(CPoint(214 - 118, 450 - 2  + 34 + doorYOff), 104, this, SN_DT, -1);
	_controls[index] = dtSlider;	
	_snControls.push_back(dtSlider);
	dtSlider->setMin(0);
	dtSlider->setMax((float)YM2612Param_getRange(SN_DT));
	dtSlider->setValue(getCurrentPatch()->getFromBaron(baron->getIndex(index)));
	dtSlider->setWheelInc(1.0f / (float)YM2612Param_getRange(SN_DT));
	frame->addView(dtSlider);





	int drumButtonsX = 12 + 20;
	int drumButtonsY = 436 + drumyOffset + 15;

	_drumLabel = new CTextLabel(CRect(drumButtonsX, drumButtonsY, drumButtonsX + 162, drumButtonsY + 18), "Click Note To Load Sample.");
	_drumLabel->setFont(kNormalFont);
	_drumLabel->setHoriAlign(kLeftText);
	_drumLabel->getFont()->setStyle(kBoldFace);
	_drumLabel->setFontColor(CColor(16, 20, 16, 255));
	_drumLabel->setMouseableArea(CRect());
	_drumLabel->setBackColor(CColor(0, 0, 0, 0));
	_drumLabel->setFrameColor(CColor(0, 0, 0, 0));
	frame->addView(_drumLabel);

	drumButtonsX += 0;
	drumButtonsY += 99; 

	int selectedDrum = ((GennyPatch*)getCurrentPatch())->InstrumentDef.SelectedDrum;
	for(int xpos = 0; xpos < 4; xpos++)
	{
		for(int ypos = 0; ypos < 5; ypos++)
		{
			int bX = drumButtonsX + (xpos * (42 + 8));
			int bY = drumButtonsY + (ypos * (14 + 2));

			int drumIndex = (xpos * 5) + (ypos);
			WaveData* drum = ((GennyPatch*)getCurrentPatch())->InstrumentDef.Drumset.getDrum(36 + drumIndex);

			CKickButton* drumButton = new CKickButton(CRect(bX + 4, bY, bX + 4 + 40, bY + 14), this, kDrumButtonIndexStart + ((xpos * 5) + (ypos)), 14, UIBitmap(IDB_PNG18), CPoint(xpos * 40, /*(drum != nullptr ? (drumIndex == selectedDrum ? 280 : 0) : 140) + */(ypos * 28)));

			UIImage* drumLight = new UIImage(CRect(bX, bY, bX + 2, bY + 14), IDB_PNG19, false);
			drumLight->setFrame(4);

			_drumButtons.push_back(drumButton);
			_drumLights.push_back(drumLight);
			_drumLightVals.push_back(0.0f);

			frame->addView(drumButton);
			frame->addView(drumLight);
		} 
	}

	_wave = new UIWaveForm(CPoint(drumButtonsX , drumButtonsY - 82), this);
	frame->addView(_wave);

	//_sampleRangeLow = new UISampleRangeSlider(CPoint(drumButtonsX , drumButtonsY - 10), 156, this, true);
	//frame->addView(_sampleRangeLow);


	_sampleRangeLow = new UIRangeSlider(drumsPos + CPoint(20, 64), 166, this, this, true, true);
	frame->addView(_sampleRangeLow);
	_sampleRangeLow->setValue(0);
	_sampleRangeLow->_slider->setTag(-1);

	_sampleRangeHigh = new UIRangeSlider(drumsPos + CPoint(20, 64), 166, this, this, false, true);
	frame->addView(_sampleRangeHigh);
	_sampleRangeHigh->setValue(0);
	_sampleRangeHigh->_slider->setTag(-1);

	_sampleRangeLow->_slider->_drawLineTo = _sampleRangeHigh->_slider;
	_sampleRangeLow->_slider->lineFromOffset = CPoint(2, -2);
	_sampleRangeLow->_slider->lineToOffset = CPoint(0, 6);

	_sampleRangeLow->_slider->_rightLimiter = _sampleRangeHigh->_slider;
	_sampleRangeHigh->_slider->_leftLimiter = _sampleRangeLow->_slider;


	_dacSamplerate = new UIDigitKnob(drumsPos + CPoint(40, 242), this, YM2612Param::YM_NONE, GIP_None, true);
	frame->addView(_dacSamplerate);
	_dacSamplerate->setVisible(false);



	CPoint dsLabelPos = drumsPos + CPoint(170 - 70, 78);
	_drumSampleHighLabel = new CTextLabel(CRect(dsLabelPos.x, dsLabelPos.y, dsLabelPos.x + 70, dsLabelPos.y + 12), "999999");
	_drumSampleHighLabel->setFont(kNormalFontBig);
	_drumSampleHighLabel->getFont()->setStyle(kBoldFace);
	_drumSampleHighLabel->setFontColor(CColor(16, 20, 16, 255));
	_drumSampleHighLabel->setMouseableArea(CRect());
	_drumSampleHighLabel->setBackColor(CColor(0, 0, 0, 0));
	_drumSampleHighLabel->setFrameColor(CColor(0, 0, 0, 0));
	_drumSampleHighLabel->setHoriAlign(CHoriTxtAlign::kRightText);
	frame->addView(_drumSampleHighLabel);
	_drumSampleHighLabel->setVisible(false);

	dsLabelPos = drumsPos + CPoint(20, 78);
	_drumSampleLowLabel = new CTextLabel(CRect(dsLabelPos.x, dsLabelPos.y, dsLabelPos.x + 70, dsLabelPos.y + 12), "999999");
	_drumSampleLowLabel->setFont(kNormalFontBig);
	_drumSampleLowLabel->getFont()->setStyle(kBoldFace);
	_drumSampleLowLabel->setFontColor(CColor(16, 20, 16, 255));
	_drumSampleLowLabel->setMouseableArea(CRect());
	_drumSampleLowLabel->setBackColor(CColor(0, 0, 0, 0));
	_drumSampleLowLabel->setFrameColor(CColor(0, 0, 0, 0));
	_drumSampleLowLabel->setHoriAlign(CHoriTxtAlign::kLeftText);
	frame->addView(_drumSampleLowLabel);
	_drumSampleLowLabel->setVisible(false);



	//_sampleRangeHigh = new UISampleRangeSlider(CPoint(drumButtonsX , drumButtonsY - 10), 156, this, false);
	//frame->addView(_sampleRangeHigh);

	rangeTop = drumButtonsY - 10;
	rangeBottom = (drumButtonsY - 10) + 10;


	WaveData* selDrum = ((GennyPatch*)getCurrentPatch())->InstrumentDef.Drumset.getDrum(36 + selectedDrum);

	int sButtonX = drumsPos.x + 198;
	int sButtonY = drumsPos.y + 32;
	_drumChL = new UICheckbox(CRect(sButtonX, sButtonY, sButtonX + 16, sButtonY + 16), this, kSamplePanningMessage, "", _owner, new CBitmap((CResourceDescription)PNG_LCHANNEL));
	_drumControls.push_back(_drumChL);
	frame->addView(_drumChL);


	sButtonX = drumsPos.x + 198;
	sButtonY = drumsPos.y + 50;
	_drumChR = new UICheckbox(CRect(sButtonX, sButtonY, sButtonX + 16, sButtonY + 16), this, kSamplePanningMessage, "", _owner, new CBitmap((CResourceDescription)PNG_RCHANNEL));
	_drumControls.push_back(_drumChR);
	frame->addView(_drumChR);

	sButtonX = drumsPos.x + 176;
	sButtonY = drumsPos.y + 68;
	UICheckbox* sampleRepeat = new UICheckbox(CRect(sButtonX, sButtonY, sButtonX + 38, sButtonY + 20), this, kSamplePanningMessage, "", _owner, new CBitmap((CResourceDescription)IDB_PNG26));
	_sampleLoop = sampleRepeat;

	if(selDrum != nullptr)
		sampleRepeat->setValue(selDrum->loop);

	_drumControls.push_back(sampleRepeat);
	frame->addView(sampleRepeat);


	_samplePitchSlider = new UIPanSlider(drumsPos + CPoint(76, 96), 112, this, this, UIPanSliderType::SamplePitch);
	frame->addView(_samplePitchSlider);
	_samplePitchSlider->setVisible(false);
	_samplePitchSlider->_slider->setTag(-1);

	CRect svsSize = CRect(0, 0, 20, 34);
	svsSize.offset(drumsPos + CPoint(176, 32));



	_sampleVolumeBacker = new UIImage(svsSize, PNG_SAMPLEVOLUMEBAR);
	_sampleVolumeBacker->setFrame(0);
	frame->addView(_sampleVolumeBacker);
	_sampleVolumeBacker->setVisible(false);
	_sampleVolumeBacker->setMouseEnabled(false);



	_sampleVolumeSlider = new UISlider(svsSize, this, 9999999, svsSize.top, svsSize.bottom - 6, new CBitmap((CResourceDescription)PNG_SLIDERWIDE), nullptr, getInterface(), CPoint(0, 0), CSliderBase::Style::kVertical | CSliderBase::Style::kBottom);
	frame->addView(_sampleVolumeSlider);
	_sampleVolumeSlider->setVisible(false);

	svsSize = CRect(0, 0, 194, 76);
	svsSize.offset(drumsPos + CPoint(20, 32));
	_drumSettingsCover = new UIImage(svsSize, PNG_DRUMSETTINGSCOVER);
	frame->addView(_drumSettingsCover);
	_drumSettingsCover->setVisible(false);
	_drumSettingsCover->setMouseEnabled(true);


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

void UIInstrument::midiLearn(int index)
{
	//std::map<int, CControl*>::iterator it = _controls.find(index);
	//if (it != _controls.end())
	//{
	//	_controls[index]->midiLearn();
	//	//if(index == _pitchIndex)
	//	//	refreshPitchDigits();

	//}
	//else
	//{
	//	std::map<int, CControl*>::iterator it = _globalControls.find(index);
	//	if (it != _globalControls.end())
	//	{
	//		_globalControls[index]->midiLearn();
	//		//if(index == _pitchIndex)
	//		//	refreshPitchDigits();

	//	}
	//}
}

void UIInstrument::midiForget(int index)
{
	//std::map<int, CControl*>::iterator it = _controls.find(index);
	//if (it != _controls.end())
	//{
	//	_controls[index]->midiForget();
	//	//if(index == _pitchIndex)
	//	//	refreshPitchDigits();

	//}
	//else
	//{
	//	std::map<int, CControl*>::iterator it = _globalControls.find(index);
	//	if (it != _globalControls.end())
	//	{
	//		_globalControls[index]->midiForget();
	//		//if(index == _pitchIndex)
	//		//	refreshPitchDigits();

	//	}
	//}
}



void UIInstrument::makeOperatorDirty(int op) 
{ 
	if (op == 1)
		op = 2;
	else if (op == 2)
		op = 1;

	_operators[op]->setDirty(true); 
}

void UIInstrument::valueChanged (CControl* control)
{
	/*if (control == _rangeSliderLow)
	{
	if (control->getValue() > _rangeSliderHigh->getValue())
		control->setValue(_rangeSliderHigh->getValue());
	}
	else if (control == _rangeSliderHigh)
	{
	if (control->getValue() < _rangeSliderLow->getValue())
		control->setValue(_rangeSliderLow->getValue());
	}

	if (control == _rangeSliderHigh || control == _rangeSliderLow)
	{
		char highBuf[4];
		char lowBuf[4];
		itoa((int)(int)(_rangeSliderHigh->getValue() + 0.5f), lowBuf, 10);
		itoa((int)(int)(_rangeSliderLow->getValue() + 0.5f), highBuf, 10);
		_rangeLabel->setText((std::string(highBuf) + "-" + std::string(lowBuf)).c_str());
		_rangeLabel->invalid();

		getOwner()->getOwner()->valueChanged(control);
		return;
	}*/

	if (control->getExtParam() != nullptr)
	{
		_owner->valueChanged(control);
		if (control == _ch3SpecialBox)
			_owner->reconnect();

		return;
	}

	if (control->getTag() == kTrueStereoIndex)
	{
		IndexBaron* baron = getIndexBaron();
		baron->enableTrueStereo = control->getValue() > 0.5f ? true : false;
		return;
	}
	else if(control == _sampleRangeLow)
	{
		if (control->getValue() > _sampleRangeHigh->getValue())
			control->setValue(_sampleRangeHigh->getValue());

		WaveData* wave = ((GennyPatch*)getCurrentPatch())->InstrumentDef.Drumset.getDrum(36 + ((GennyPatch*)getCurrentPatch())->InstrumentDef.SelectedDrum);
		if (wave != nullptr)
		{
			wave->setStartSample((int)(control->getValue() * wave->originalDataSize));

			char buf[10];
			itoa(wave->_originalStartSample, buf, 10);
			_drumSampleLowLabel->setText(buf);
		}

		_wave->invalid();
		return;
	}
	else if(control == _sampleRangeHigh)
	{
		if (control->getValue() < _sampleRangeLow->getValue())
			control->setValue(_sampleRangeLow->getValue());

		WaveData* wave = ((GennyPatch*)getCurrentPatch())->InstrumentDef.Drumset.getDrum(36 + ((GennyPatch*)getCurrentPatch())->InstrumentDef.SelectedDrum);
		if (wave != nullptr)
		{
			wave->setEndSample((int)(control->getValue() * wave->originalDataSize));

			char buf[10];
			itoa(wave->_originalEndSample, buf, 10);
			_drumSampleHighLabel->setText(buf);
		}

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
	else if (control == _drumChL)
	{
		WaveData* wave = ((GennyPatch*)getCurrentPatch())->InstrumentDef.Drumset.getDrum(36 + ((GennyPatch*)getCurrentPatch())->InstrumentDef.SelectedDrum);
		if (wave != nullptr)
			wave->panLeft = _drumChL->getValue() > 0.3f;

		getInterface()->valueChanged(control);
		return;
	}
	else if (control == _drumChR)
	{
		WaveData* wave = ((GennyPatch*)getCurrentPatch())->InstrumentDef.Drumset.getDrum(36 + ((GennyPatch*)getCurrentPatch())->InstrumentDef.SelectedDrum);
		if (wave != nullptr)
			wave->panRight = _drumChR->getValue() > 0.3f;

		getInterface()->valueChanged(control);
		return;
	}
	else if (control == _samplePitchSlider)
	{
		WaveData* wave = ((GennyPatch*)getCurrentPatch())->InstrumentDef.Drumset.getDrum(36 + ((GennyPatch*)getCurrentPatch())->InstrumentDef.SelectedDrum);
		if (wave != nullptr)
			wave->setPitch((int)(_samplePitchSlider->getValue() * 1200) - 600);

		return;
	}
	else if (control == _sampleVolumeSlider)
	{
		WaveData* wave = ((GennyPatch*)getCurrentPatch())->InstrumentDef.Drumset.getDrum(36 + ((GennyPatch*)getCurrentPatch())->InstrumentDef.SelectedDrum);
		if (wave != nullptr)
		{
			wave->setAmp((int)(_sampleVolumeSlider->getValue() * 100));
			_sampleVolumeBacker->setFrame(min((int)((((wave->_amp / 100.0f)) * 17) + 0.5f), 17));
		}

		return;
	}
	//if(control->getTag() == _pitchIndex)
	//	refreshPitchDigits();

	if(control->getTag() >= kDrumButtonIndexStart && control->getTag() <= kDrumButtonIndexEnd)
	{
		int drumIndex = control->getTag() - kDrumButtonIndexStart;
		WaveData* drum = ((GennyPatch*)getCurrentPatch())->InstrumentDef.Drumset.getDrum(36 + drumIndex);
		if(drum == nullptr || (_lastDrumClick > 0.0f && _lastDrumClickTag == control->getTag()))
		{	
			if(control->getValue() == 1.0f)
				_owner->openImportSample(control->getTag() - kDrumButtonIndexStart);

			drum = ((GennyPatch*)getCurrentPatch())->InstrumentDef.Drumset.getDrum(36 + drumIndex);
		}

		_lastDrumClickTag = control->getTag();

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

int32_t UIInstrument::controlModifierClicked(VSTGUI::CControl* pControl, VSTGUI::CButtonState button)
{
	if (pControl->getTag() >= kDrumButtonIndexStart && pControl->getTag() <= kDrumButtonIndexEnd && button.isRightButton() )
	{
		int drumIndex = pControl->getTag() - kDrumButtonIndexStart;
		WaveData* drum = ((GennyPatch*)getCurrentPatch())->InstrumentDef.Drumset.getDrum(36 + drumIndex);
		_owner->openImportSample(pControl->getTag() - kDrumButtonIndexStart);
		return 1;
	}
	return 0;
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
		//Global controls take their settings from patch ZERO
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

	WaveData* wave = ((GennyPatch*)getCurrentPatch())->InstrumentDef.Drumset.getDrum(36 + ((GennyPatch*)getCurrentPatch())->InstrumentDef.SelectedDrum);
	if(_drumDoor->isVisible())
	{
		_dacSamplerate->setVisible(true);
		_wave->setVisible(wave != nullptr);
		_wave->setDirty(true);
		_drumLabel->setVisible(true);
		_sampleRangeHigh->setVisible(wave != nullptr);
		_sampleRangeLow->setVisible(wave != nullptr);

		_drumSampleHighLabel->setVisible(wave != nullptr);
		_drumSampleLowLabel->setVisible(wave != nullptr);
		_samplePitchSlider->setVisible(wave != nullptr);
		_sampleVolumeSlider->setVisible(wave != nullptr);
		_sampleVolumeBacker->setVisible(wave != nullptr);

		_sampleLoop->setVisible(wave != nullptr);
		_drumChR->setVisible(wave != nullptr);
		_drumChL->setVisible(wave != nullptr);
		_drumSettingsCover->setVisible(wave == nullptr);
	}	
	else
	{
		_dacSamplerate->setVisible(false);
		_drumLabel->setVisible(false);
		_wave->setVisible(false);
		_wave->setDirty(true);
		_sampleRangeHigh->setVisible(false);
		_sampleRangeLow->setVisible(false);

		_drumSampleHighLabel->setVisible(false);
		_drumSampleLowLabel->setVisible(false);
		_samplePitchSlider->setVisible(false);
		_sampleVolumeSlider->setVisible(false);
		_sampleVolumeBacker->setVisible(false);

		_sampleLoop->setVisible(false);
		_drumChR->setVisible(false);
		_drumChL->setVisible(false);
		_drumSettingsCover->setVisible(false);
	}

	_specialMode->setValue(baron->enableTrueStereo ? 1.0f : 0.0f);

	if(wave != nullptr)
	{
		_sampleLoop->setValue(wave->loop ? 1.0f : 0.0f);
		_drumChR->setValue(wave->panRight ? 1.0f : 0.0f);
		_drumChL->setValue(wave->panLeft ? 1.0f : 0.0f);
	}

	_samplePitchSlider->_slider->setMax(1.0f);
	_samplePitchSlider->_slider->setWheelInc(1.0f / 2400);
	_samplePitchSlider->_slider->centerStick = 0.5f;

	_sampleVolumeSlider->setMax(1.0f);
	_sampleVolumeSlider->setWheelInc(1.0f / 100);
	_sampleVolumeSlider->centerStick = 0.5f;
	

	if (wave != nullptr)
	{
		_sampleRangeLow->_slider->setMax(1.0f);
		_sampleRangeLow->_slider->setWheelInc(1.0f / wave->originalDataSize);
		_sampleRangeLow->_slider->setValue(wave->_originalStartSample / wave->originalDataSize);
		_sampleRangeLow->_slider->setTag(-1);
		char buf[10];
		itoa(wave->_originalStartSample, buf, 10);
		_drumSampleLowLabel->setText(buf);
		_drumSampleLowLabel->invalid();

		_sampleRangeHigh->_slider->setMax(1.0f);
		_sampleRangeHigh->_slider->setWheelInc(1.0f / wave->originalDataSize);
		_sampleRangeHigh->_slider->setValue(wave->_originalEndSample / wave->originalDataSize);
		itoa(wave->_originalEndSample, buf, 10);
		_drumSampleHighLabel->setText(buf);
		_drumSampleHighLabel->invalid();

		_samplePitchSlider->_slider->setValue((wave->_pitch + 600.0f) / 1200.0f);
		_sampleVolumeSlider->setValue(wave->_amp / 100.0f);
		_sampleVolumeBacker->setFrame(min((int)((wave->_amp / 100.0f) * 19), 17));
	}
	else
	{
		_drumLabel->setText("Click Note To Load Sample.");
	}

	_samplePitchSlider->updateLabel();

	_ch3SpecialBox->setExtParam(getCurrentPatch()->getExt(GEParam::Op3Special));
	_lfoEnCheckbox->setExtParam(getCurrentPatch()->getExt(GEParam::LFOEnable));

	_dacSamplerate->setExtParam(getCurrentPatch()->getExt(GEParam::DACSamplerate));

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
			int buttonY = (_drumButtons[i]->getTag() - kDrumButtonIndexStart) % 5;

			_drumButtons[i]->setBackground(UIBitmap(IDB_PNG18));
			_drumButtons[i]->setSpriteSheetOffsetAdd(CPoint(0, (drum != nullptr ? (drumIndex == selectedDrum ? 280 : 0) : 140)));

			_drumButtons[i]->setVisible(false);
			_drumButtons[i]->setVisible(true);
			_drumButtons[i]->invalid();
			_drumButtons[i]->setDirty(true);
			 
			if(drum != nullptr && drumIndex == selectedDrum)
			{
				std::string labe = std::string(drum->sampleName) + std::string(" (") + std::to_string(drum->originalSampleRate) + std::string("hz)");
				_drumLabel->setText(labe.c_str());
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

	_lfoKnob->setValue(getVst()->lfo);
	//_lfoKnob->setValue()

	_operators[0]->reconnect(_drumDoor->isVisible() || isFM <= 0);
	_operators[1]->reconnect(_drumDoor->isVisible() || isFM <= 0);
	_operators[2]->reconnect(_drumDoor->isVisible() || isFM <= 0);
	_operators[3]->reconnect(_drumDoor->isVisible() || isFM <= 0);

	int dtIndex = getIndexBaron()->getYMParamIndex(SN_DT);
	_controls[dtIndex]->setValue(getCurrentPatch()->getFromBaron(baron->getIndex(dtIndex)));

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
		_lastDrumClick -= 0.08f;
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
