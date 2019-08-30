#include "UIPESelectedInstrument.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIInstrumentsPanel.h"
#include "UIMidiChannel.h"
#include "UIPanSlider.h"
#include "UIRangeSlider.h"
#include "UISpinner.h"
#include "UICheckbox.h"
#include "UICheckBoxNum.h"

const int kRadioFMOn = 949304;
const int kRadioFMOff = 949305;
const int kRadioDrumOn = 949306;
UIPESelectedInstrument::UIPESelectedInstrument(UIInstrumentsPanel* vOwner, GennyPatch* vPatch, int vIndex):
	GennyInterfaceObject(vOwner),
	_owner(vOwner),
	_selected(false),
	_patch(vPatch)
{
	CFrame* frame = vOwner->getFrame();

	IndexBaron* baron = getIndexBaron();
	UIBitmap buttonImage(PNG_INSTRUMENTCHECKBOX);
	for(int i = 0; i < 6; i++)
	{
		int index = baron->getInsParamIndex((GennyInstrumentParam)(GIP_Channel0 + i));

		CRect buttonSize = CRect(0, 0, 20, 18);
		buttonSize.offset(606 + (i * 18), 136);
		_channels.push_back(new UICheckBoxNum(buttonSize, i, this, i, buttonImage, _interface, true));
		frame->addView(_channels[i]);
		_channels[i]->setTag(index);
	}

	CRect buttonSize = CRect(0, 0, 106, 18);
	buttonSize.offset(606 + (0 * 18), 136);
	_channel6 = new UICheckBoxNum(buttonSize, 5, this, 5, UIBitmap(PNG_CHANNEL6), _interface, true);
	frame->addView(_channel6);
	_channel6->setTag(baron->getInsParamIndex((GennyInstrumentParam)(GIP_Channel0 + 5)));
	_channel6->setVisible(false);

	for(int i = 0; i < 4; i++)
	{
		int index = baron->getInsParamIndex((GennyInstrumentParam)(GIP_Channel0 + i + 6));
		
		CRect buttonSize;
		if(i == 3)
		{
			buttonSize = CRect(0, 0, 60, 18);
			buttonSize.offset(606 + (i * 18), 136);
			_channels.push_back(new UICheckbox(buttonSize, this, _channels.size() - 1, "", nullptr, UIBitmap(PNG_NOISECHECKBOX)));
			frame->addView(_channels[_channels.size() - 1]);
			_channels[_channels.size() - 1]->setTag(index);
		}
		else
		{
			buttonSize = CRect(0, 0, 20, 18);
			buttonSize.offset(606 + (i * 18), 136);
			_channels.push_back(new UICheckBoxNum(buttonSize, i, this, _channels.size() - 1, buttonImage, _interface, true));
			frame->addView(_channels[_channels.size() - 1]);
			_channels[_channels.size() - 1]->setTag(index);
		}


	}

	_channelSelector = new UIMidiChannel(vOwner);
	frame->addView(_channelSelector);

	_octaveSelector = new UISpinner(CPoint(618, 218 - 16), vOwner, GIP_Octave);
	frame->addView(_octaveSelector);
	_transposeSelector = new UISpinner(CPoint(618 + 54, 218 - 16), vOwner, GIP_Transpose);
	frame->addView(_transposeSelector);
	_panSlider = new UIPanSlider(CPoint(650, 262 + 4), 60, this); 
	frame->addView(_panSlider);
	_delaySlider = new UIPanSlider(CPoint(650 - 4, 262 + 4 - 14), 46, this, true);
	frame->addView(_delaySlider);

	UIBitmap radioImage(PNG_FMBUTTON);
	CRect radioSize = CRect(0, 0, 42, 18);
	radioSize.offset(606, 100);
	_fmSelectors.push_back(new UICheckbox(radioSize, this, kRadioFMOn, "", nullptr, radioImage));
	frame->addView(_fmSelectors[0]);

	UIBitmap radioImage2(PNG_PSGBUTTON);
	radioSize = CRect(0, 0, 38, 18);
	radioSize.offset(606 + 44, 100);
	_fmSelectors.push_back(new UICheckbox(radioSize, this, kRadioFMOff, "", nullptr, radioImage2));
	frame->addView(_fmSelectors[1]);

	UIBitmap radioImage3(IDB_PNG17);
	radioSize = CRect(0, 0, 22, 18);
	radioSize.offset(606 + 46 + 38, 100);
	_fmSelectors.push_back(new UICheckbox(radioSize, this, kRadioDrumOn, "", nullptr, radioImage3));
	frame->addView(_fmSelectors[2]);

	_rangeSliderLow = new UIRangeSlider(CPoint(650 - 42, 262 - 22 - 8), 60 + 50, this, false);
	frame->addView(_rangeSliderLow);
	_rangeSliderLow->setValue(1);
	_rangeSliderHigh = new UIRangeSlider(CPoint(650 - 42, 262 - 22), 60 + 50, this, true);
	frame->addView(_rangeSliderHigh);
	_rangeSliderHigh->setValue(0);

	int labelX = 646;
	int labelY = 262 - 22 - 18 + 2;
	_rangeLabel = new CTextLabel(CRect(labelX, labelY, labelX + 60, labelY + 12), "127-127");
	_rangeLabel->setFont(kNormalFontBig);
	_rangeLabel->setHoriAlign(kLeftText);
	_rangeLabel->getFont()->setStyle(kBoldFace);
	_rangeLabel->setFontColor(CColor(16, 20, 16, 255));
	_rangeLabel->setMouseableArea(CRect());
	_rangeLabel->setBackColor(CColor(0, 0, 0, 0));
	_rangeLabel->setFrameColor(CColor(0, 0, 0, 0));
	_rangeLabel->setHoriAlign(CHoriTxtAlign::kRightText);
	frame->addView(_rangeLabel);

	reconnect();

	setPatchLink(vPatch);
}

UIPESelectedInstrument::~UIPESelectedInstrument(void)
{

}

void UIPESelectedInstrument::valueChanged (CControl* control)
{
	if(control == _rangeSliderHigh)
	{
		if(control->getValue() > _rangeSliderLow->getValue())
			control->setValue(_rangeSliderLow->getValue());
	}
	else if(control == _rangeSliderLow)
	{
		if(control->getValue() < _rangeSliderHigh->getValue())
			control->setValue(_rangeSliderHigh->getValue());
	}

	if(control == _rangeSliderHigh || control == _rangeSliderLow)
	{
		char highBuf[4];
		itoa((int)(int)(_rangeSliderHigh->getValue() + 0.5f), highBuf, 10);
		char lowBuf[4];
		itoa((int)(int)(_rangeSliderLow->getValue() + 0.5f), lowBuf, 10);
		_rangeLabel->setText((std::string(highBuf) + "-" + std::string(lowBuf)).c_str());
		_rangeLabel->invalid();

		getOwner()->getOwner()->valueChanged(control);
		return;
	}

	if(control->getTag() == kRadioFMOn)
	{
		//Toggle FM mode
		if(control->getValue() == 1.0f)
		{
			int myTag = control->getTag();

			IndexBaron* baron = getIndexBaron();
			int index = baron->getInsParamIndex(GIP_FM);
			control->setTag(index);
			control->setValue(1.0f);
			_patch->InstrumentDef.Type = GIType::FM;

			_owner->getOwner()->valueChanged(control);
			control->setTag(myTag);


			std::vector<VSTPatch*> patches = getVst()->getPatches();
			std::vector<int> indexes;
			for(int i = 0; i < 16; i++)
			{
				int index = baron->getPatchParamIndex((GennyPatchParam)(GPP_Ins01 + i));
				int idx = (int)getVst()->getPatch(0)->getFromBaron(baron->getIndex(index));
				indexes.push_back(idx);
			}


			for(int i = 0; i < indexes.size(); i++)
			{
				if(indexes[i] == getVst()->getPatchIndex(_patch))
				{
					for(int j = 0; j < 10; j++)
					{
						if(true)
							getVst()->getPatch(i)->setFromBaron(baron->getIndex(baron->getInsParamIndex((GennyInstrumentParam)(GIP_Channel0 + j))), j < 6 ? 1.0f : 0.0f);
						else
							getVst()->getPatch(i)->setFromBaron(baron->getIndex(baron->getInsParamIndex((GennyInstrumentParam)(GIP_Channel0 + j))), (j > 5 && j < 9) ? 1.0f : 0.0f);
					}
				}
			}

			_fmSelectors[1]->setValue(0.0f);
			_fmSelectors[2]->setValue(0.0f);
			getInterface()->reconnect();
		}
		else
		{
			control->setValue(1.0f);
		}
	}
	else if(control->getTag() == kRadioFMOff)
	{
		if(control->getValue() == 1.0f)
		{
			int myTag = control->getTag();
			IndexBaron* baron = getIndexBaron();

			int index = baron->getInsParamIndex(GIP_FM);
			control->setTag(index);
			control->setValue(0.0f);
			_patch->InstrumentDef.Type = GIType::SN;

			_owner->getOwner()->valueChanged(control);
			control->setTag(myTag);
			control->setValue(1.0f);


			std::vector<VSTPatch*> patches = getVst()->getPatches();
			std::vector<int> indexes;
			for(int i = 0; i < 16; i++)
			{
				int index = baron->getPatchParamIndex((GennyPatchParam)(GPP_Ins01 + i));
				int idx = (int)getVst()->getPatch(0)->getFromBaron(baron->getIndex(index));
				indexes.push_back(idx);
			}


			for(int i = 0; i < indexes.size(); i++)
			{
				if(indexes[i] == getVst()->getPatchIndex(_patch))
				{
					for(int j = 0; j < 10; j++)
					{
						if(false)
							getVst()->getPatch(i)->setFromBaron(baron->getIndex(baron->getInsParamIndex((GennyInstrumentParam)(GIP_Channel0 + j))), j < 6 ? 1.0f : 0.0f);
						else
							getVst()->getPatch(i)->setFromBaron(baron->getIndex(baron->getInsParamIndex((GennyInstrumentParam)(GIP_Channel0 + j))), (j > 5 && j < 9) ? 1.0f : 0.0f);
					}
				}
			}


			_fmSelectors[0]->setValue(0.0f);
			_fmSelectors[2]->setValue(0.0f);
			getInterface()->reconnect();
		}
		else
		{
			control->setValue(1.0f);
		}
	}
	else if(control->getTag() == kRadioDrumOn)
	{
		if(control->getValue() == 1.0f)
		{
			int myTag = control->getTag();
			IndexBaron* baron = getIndexBaron();

			_patch->setFromBaron(baron->getIndex(baron->getInsParamIndex(GIP_FM)), 1.0f);

			int index = baron->getInsParamIndex(GIP_DAC);
			control->setTag(index);
			control->setValue(1.0f);
			_patch->InstrumentDef.Type = GIType::DAC;
			_patch->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex((YM2612Param)YM_DRUMTL, 3)), YM2612Param_getRange(YM_DRUMTL) - 27);

			_owner->getOwner()->valueChanged(control);
			control->setTag(myTag);
			control->setValue(1.0f);





			std::vector<VSTPatch*> patches = getVst()->getPatches();
			std::vector<int> indexes;
			for(int i = 0; i < 16; i++)
			{
				int index = baron->getPatchParamIndex((GennyPatchParam)(GPP_Ins01 + i));
				int idx = (int)getVst()->getPatch(0)->getFromBaron(baron->getIndex(index));
				indexes.push_back(idx);
			}


			for(int i = 0; i < indexes.size(); i++)
			{
				if(indexes[i] == getVst()->getPatchIndex(_patch))
				{
					for(int j = 0; j < 10; j++)
					{
						getVst()->getPatch(i)->setFromBaron(baron->getIndex(baron->getInsParamIndex((GennyInstrumentParam)(GIP_Channel0 + j))), (j == 5) ? 1.0f : 0.0f);
					}
				}
			}

			_fmSelectors[0]->setValue(0.0f);
			_fmSelectors[1]->setValue(0.0f);
			getInterface()->reconnect();
		}
		else
		{
			control->setValue(1.0f);
		}
	}
	else if(control->getTag() == 999999)
	{

	}
	else
	{
		for(int i = 0; i < _channels.size(); i++)
		{
			if(_channels[i] == control)
			{
				getInterface()->valueChanged(control);
				
				_owner->makeChannelsDirty();
				return;
			}
		}

		if(control == _channel6)
		{
			getInterface()->valueChanged(control);
			
			_owner->makeChannelsDirty();
			return;
		}

		_owner->valueChanged(control);
	}
}


void UIPESelectedInstrument::setPatchLink(GennyPatch* patch)
{
	_patch = patch;
	if(_patch != nullptr)
	{
		//_label->setText(patch->Name.c_str());

		IndexBaron* baron = _owner->getIndexBaron();
		std::vector<VSTPatch*> patches = getVst()->getPatches();
		std::vector<int> indexes;
		for(int i = 0; i < 16; i++)
		{
			int index = baron->getPatchParamIndex((GennyPatchParam)(GPP_Ins01 + i));
			int idx = (int)getVst()->getPatch(0)->getFromBaron(baron->getIndex(index));
			indexes.push_back(idx);
		}
	}
}

void UIPESelectedInstrument::reconnect()
{
	IndexBaron* baron = getIndexBaron(); 

	int selection = getInstrumentIndex(getPatch(0)->SelectedInstrument);
	int fmEn = 0;
	if(_patch != nullptr)
		fmEn = (int)(_patch->InstrumentDef.Type == GIType::FM || _patch->InstrumentDef.Type == GIType::DAC);

	GennyPatch* selectedPatch = (GennyPatch*)getVst()->getPatch(selection);

	int inder = 0;
	for(int i = 0; i < 6; i++)
	{
		_channels[i]->setValue(selectedPatch->InstrumentDef.Channels[i]);
		_channels[i]->setTag(kChannelEnableStart + i);

		if(fmEn == 1 && _patch != nullptr && _patch->InstrumentDef.Type != GIType::DAC)
		{
			_channels[i]->setVisible(true);
			_channels[i]->invalid();
		}
		else
		{
			_channels[i]->setVisible(false);
			_channels[i]->invalid();
		}
	}
	for(int i = 0; i < 4; i++)
	{
		_channels[i + 6]->setValue(selectedPatch->InstrumentDef.Channels[i + 6]);
		_channels[i + 6]->setTag(kChannelEnableStart + i + 6);

		if(fmEn == 0 && _patch != nullptr && _patch->InstrumentDef.Type != GIType::DAC)
		{
			_channels[i + 6]->setVisible(true);
			_channels[i + 6]->invalid();
		}
		else
		{
			_channels[i + 6]->setVisible(false);
			_channels[i + 6]->invalid();
		}
	}

	if(_patch != NULL && _patch->InstrumentDef.Type != GIType::DAC)
	{
		if(fmEn == 0)
		{
			_fmSelectors[0]->setValue(0.0f);
			_fmSelectors[1]->setValue(1.0f);
			_fmSelectors[2]->setValue(0.0f);
		}
		else
		{
			_fmSelectors[0]->setValue(1.0f);
			_fmSelectors[1]->setValue(0.0f);
			_fmSelectors[2]->setValue(0.0f);
		}
	}
	else
	{
		_fmSelectors[0]->setValue(0.0f);
		_fmSelectors[1]->setValue(0.0f);
		_fmSelectors[2]->setValue(1.0f);
	}

	_channelSelector->reconnect();
	_octaveSelector->reconnect();
	_transposeSelector->reconnect();
	_panSlider->reconnect();
	_delaySlider->reconnect();
	_rangeSliderLow->reconnect();
	_rangeSliderHigh->reconnect();

	char highBuf[4];
	itoa((int)(int)_rangeSliderHigh->getValue(), highBuf, 10);
	char lowBuf[4];
	itoa((int)(int)_rangeSliderLow->getValue(), lowBuf, 10);
	_rangeLabel->setText((std::string(highBuf) + "-" + std::string(lowBuf)).c_str());

	if(_patch != nullptr)
	{
		//Jumbo Channel 6 Button
		_channel6->setValue(selectedPatch->InstrumentDef.Channels[5]);
		_channel6->setTag(kChannelEnableStart + 5);
		_channel6->setVisible(_patch->InstrumentDef.Type == GIType::DAC);
		_channel6->invalid();
	}
}