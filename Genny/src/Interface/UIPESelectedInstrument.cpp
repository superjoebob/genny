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
#include "lib/platform/win32/win32frame.h"

const int kRadioFMOn = 949304;
const int kRadioFMOff = 949305;
const int kRadioDrumOn = 949306;
UIPESelectedInstrument::UIPESelectedInstrument(UIInstrumentsPanel* vOwner, GennyPatch* vPatch, int vIndex):
	GennyInterfaceObject(vOwner),
	_owner(vOwner),
	_selected(false),
	_patch(vPatch),
	_transTab(nullptr),
	_soloCheckbox(nullptr),
	_transTabButton(nullptr),
	_moreTabButton(nullptr),
	_glideSlider(nullptr),
	_delaySlider(nullptr),
	_legatoCheckbox(nullptr),
	_togglingChannels(false)
{
	CFrame* frame = vOwner->getFrame();

	float channelsY = 120;
	IndexBaron* baron = getIndexBaron();
	UIBitmap buttonImage(PNG_INSTRUMENTCHECKBOX);
	for(int i = 0; i < 6; i++)
	{
		int index = baron->getInsParamIndex((GennyInstrumentParam)(GIP_Channel0 + i));

		CRect buttonSize = CRect(0, 0, 20, 18);
		buttonSize.offset(606 + (i * 18), channelsY);
		_channels.push_back(new UICheckBoxNum(buttonSize, i, this, i, buttonImage, _interface, true));
		frame->addView(_channels[i]);
		_channels[i]->setTag(index);
	}

	CRect buttonSize = CRect(0, 0, 106, 18);
	buttonSize.offset(606 + (0 * 18), channelsY);
	_channel6 = new UICheckBoxNum(buttonSize, 5, this, 5, UIBitmap(PNG_CHANNEL6), _interface, true);
	frame->addView(_channel6);
	_channel6->setTag(baron->getInsParamIndex((GennyInstrumentParam)(GIP_Channel0 + 5)));
	_channel6->setVisible(false);

	buttonSize = CRect(0, 0, 106, 18);
	buttonSize.offset(606 + (0 * 18), channelsY);
	_channelSp3 = new UICheckBoxNum(buttonSize, 2, this, 2, UIBitmap(PNG_CHANNEL6), _interface, true);
	frame->addView(_channelSp3);
	_channelSp3->setTag(baron->getInsParamIndex((GennyInstrumentParam)(GIP_Channel0 + 2)));
	_channelSp3->setVisible(false);


	for(int i = 0; i < 4; i++)
	{
		int index = baron->getInsParamIndex((GennyInstrumentParam)(GIP_Channel0 + i + 6));
		
		CRect buttonSize;
		if(i == 3)
		{
			buttonSize = CRect(0, 0, 36, 18);
			buttonSize.offset((606 + (i * 18)) + 16, channelsY);
			_channels.push_back(new UICheckbox(buttonSize, this, _channels.size() - 1, "", _interface, UIBitmap(PNG_NOISECHECKBOX)));
			frame->addView(_channels[_channels.size() - 1]);
			_channels[_channels.size() - 1]->setTag(index);

			buttonSize = CRect(0, 0, 18, 18);
			buttonSize.offset(606 + (i * 18) - 2, channelsY);
			_channels.push_back(new UICheckbox(buttonSize, this, _channels.size() - 1, "", _interface, UIBitmap(IDB_PNG41)));
			frame->addView(_channels[_channels.size() - 1]);
			_channels[_channels.size() - 1]->setTag(index);
		}
		else
		{
			buttonSize = CRect(0, 0, 20, 18);
			buttonSize.offset(606 + (i * 18), channelsY);
			_channels.push_back(new UICheckBoxNum(buttonSize, i, this, _channels.size() - 1, buttonImage, _interface, true));
			frame->addView(_channels[_channels.size() - 1]);
			_channels[_channels.size() - 1]->setTag(index);
		}
	}


	_transTab = new UIImage(CRect(606, 140, 606 + 106, 140 + 140), IDB_TRANSTAB, false);
	_transTab->setMouseEnabled(false);
	_transTab->setMouseableArea(CRect(-999, -999, -999, -999));
	frame->addView(_transTab);

	_transTabButton = new CKickButton(CRect(608, 262, 608 + 62, 262 + 17), this, 9999, 20, nullptr);
	frame->addView(_transTabButton);

	_moreTabButton = new CKickButton(CRect(672, 262, 672 + 20, 262 + 18), this, 9999, 20, nullptr);
	frame->addView(_moreTabButton);

	_delayTabButton = new CKickButton(CRect(690, 262, 690 + 20, 262 + 18), this, 9999, 20, nullptr);
	frame->addView(_delayTabButton);


	_pingPongButton = new CKickButton(CRect(610, 210, 610 + 98, 210 + 18), this, 9999, 20, nullptr);
	frame->addView(_pingPongButton);
	_pingPongButton->setVisible(false);


	_channelSelector = new UIMidiChannel(CPoint(662, 146), vOwner, vOwner, -1);
	frame->addView(_channelSelector);

	_octaveSelector = new UISpinner(CPoint(620, 186), vOwner, GIP_Octave);
	frame->addView(_octaveSelector);
	_transposeSelector = new UISpinner(CPoint(618 + 52, 186), vOwner, GIP_Transpose);
	frame->addView(_transposeSelector);


	_panSlider = new UIPanSlider(CPoint(624, 282), 76, this, this, UIPanSliderType::Pan); 
	frame->addView(_panSlider);




	UIBitmap radioImage(PNG_FMBUTTON);
	CRect radioSize = CRect(0, 0, 42, 18);
	radioSize.offset(606, 100);
	_fmSelectors.push_back(new UICheckbox(radioSize, this, kRadioFMOn, "", _interface, radioImage));
	frame->addView(_fmSelectors[0]);

	UIBitmap radioImage2(PNG_PSGBUTTON);
	radioSize = CRect(0, 0, 38, 18);
	radioSize.offset(606 + 44, 100);
	_fmSelectors.push_back(new UICheckbox(radioSize, this, kRadioFMOff, "", _interface, radioImage2));
	frame->addView(_fmSelectors[1]);

	UIBitmap radioImage3(IDB_PNG17);
	radioSize = CRect(0, 0, 22, 18);
	radioSize.offset(606 + 46 + 38, 100);
	_fmSelectors.push_back(new UICheckbox(radioSize, this, kRadioDrumOn, "", _interface, radioImage3));
	frame->addView(_fmSelectors[2]);

	_rangeSliderLow = new UIRangeSlider(CPoint(610, 220), 96, this, this, true);
	frame->addView(_rangeSliderLow);
	_rangeSliderLow->setValue(0);
	_rangeSliderHigh = new UIRangeSlider(CPoint(610, 228), 96, this, this, false);
	frame->addView(_rangeSliderHigh);
	_rangeSliderHigh->setValue(1);

	_rangeSliderLow->_slider->_drawLineTo = _rangeSliderHigh->_slider;
	_rangeSliderLow->_slider->_rightLimiter = _rangeSliderHigh->_slider;
	_rangeSliderHigh->_slider->_leftLimiter = _rangeSliderLow->_slider;


	int labelX = 648;
	int labelY = 213;
	_rangeLabel = new CTextLabel(CRect(labelX, labelY - 4, labelX + 60, labelY + 12), "127-127");
	_rangeLabel->setFont(kNormalFontBig);
	_rangeLabel->setHoriAlign(kLeftText);
	_rangeLabel->getFont()->setStyle(kBoldFace);
	_rangeLabel->setFontColor(CColor(16, 20, 16, 255));
	_rangeLabel->setMouseableArea(CRect());
	_rangeLabel->setBackColor(CColor(0, 0, 0, 0));
	_rangeLabel->setFrameColor(CColor(0, 0, 0, 0));
	_rangeLabel->setHoriAlign(CHoriTxtAlign::kRightText);
	frame->addView(_rangeLabel);

	labelX = 612;
	labelY = 212;
	_pingpongLabel = new CTextLabel(CRect(labelX, labelY, labelX + 82, labelY + 14), "127-127");
	_pingpongLabel->setFont(kNormalFontBig);
	_pingpongLabel->getFont()->setStyle(kBoldFace);
	_pingpongLabel->setFontColor(CColor(16, 20, 16, 255));
	_pingpongLabel->setMouseableArea(CRect());
	_pingpongLabel->setBackColor(CColor(0, 0, 0, 0));
	_pingpongLabel->setFrameColor(CColor(0, 0, 0, 0));
	_pingpongLabel->setHoriAlign(CHoriTxtAlign::kCenterText);
	frame->addView(_pingpongLabel);
	_pingpongLabel->setVisible(false);

	 

	buttonSize = CRect(0, 0, 54, 18);
	buttonSize.offset(610, 144);
	_soloCheckbox = new UICheckbox(buttonSize, this, 99998, "", _interface, UIBitmap(IDB_PNG47));
	frame->addView(_soloCheckbox);
	_soloCheckbox->setVisible(false);

	buttonSize = CRect(0, 0, 42, 18);
	buttonSize.offset(666, 144);
	_legatoCheckbox = new UICheckbox(buttonSize, this, 99999, "", _interface, UIBitmap(IDB_LEGATO));
	frame->addView(_legatoCheckbox);
	_legatoCheckbox->setVisible(false);


	buttonSize = CRect(0, 0, 16, 16);
	buttonSize.offset(606, 282);
	_lChannelButton = new UICheckbox(buttonSize, this, 0, "", _interface, UIBitmap(PNG_LCHANNEL));
	frame->addView(_lChannelButton);
	
	buttonSize = CRect(0, 0, 16, 16);
	buttonSize.offset(696, 282);
	_rChannelButton = new UICheckbox(buttonSize, this, 0, "", _interface, UIBitmap(PNG_RCHANNEL));
	frame->addView(_rChannelButton);


	_delaySlider = new UIPanSlider(CPoint(612, 156), 98, this, this, UIPanSliderType::Delay);
	frame->addView(_delaySlider);
	_delaySlider->setVisible(false);

	_glideSlider = new UIPanSlider(CPoint(612, 180), 98, this, this, UIPanSliderType::Glide);
	frame->addView(_glideSlider);
	_glideSlider->setVisible(false);

	_detuneSlider = new UIPanSlider(CPoint(632, 244), 60, this, this, UIPanSliderType::DetuneHor);
	frame->addView(_detuneSlider);



	reconnect();

	setPatchLink(vPatch);
}

UIPESelectedInstrument::~UIPESelectedInstrument(void)
{
	if (ContextMenu != nullptr)
	{
		// destroy our popup menu and all subitems
		int count = GetMenuItemCount((HMENU)ContextMenu);
		while (count > 0) {
			DeleteMenu((HMENU)ContextMenu, count - 1, MF_BYPOSITION);
			count--;
		}
		DestroyMenu((HMENU)ContextMenu);
	}
}




char pingpongEditString[80];
BOOL CALLBACK TypePingPongProc(HWND hwndDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemTextA(hwndDlg, IDC_EDIT1, "LLCCRR");
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			if (!GetDlgItemTextA(hwndDlg, IDC_EDIT1, pingpongEditString, 80))
				*pingpongEditString = 0;

			// Fall through. 
		case IDCANCEL:
			EndDialog(hwndDlg, wParam);
			return TRUE;
		}
	}
	return FALSE;
}


void UIPESelectedInstrument::valueChanged (CControl* control)
{
	if (control == _rangeSliderLow)
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
	}

	if (_togglingChannels == false)
	{
		_togglingChannels = true;

		bool doreconnect = false;
		for (int i = 0; i < _channels.size(); i++)
		{
			if (_channels[i] == control)
			{
				UICheckbox* box = (UICheckbox*)control;

				if (box->controlModifier)
				{
					bool allOn = true;

					for (int i2 = 0; i2 < _channels.size(); i2++)
					{
						UICheckbox* box2 = (UICheckbox*)_channels[i2];
						if (box2->isVisible())
						{
							if (box2 != box && box2->getValue() > 0.5f)
							{
								allOn = false;
								break;
							}
						}
					}

					for (int i2 = 0; i2 < _channels.size(); i2++)
					{
						UICheckbox* box2 = (UICheckbox*)_channels[i2];
						if (box2->isVisible())
						{
							if (box2 == box || allOn)
								box2->getExtParam()->set(1.0f);
							else
								box2->getExtParam()->set(0.0f);
						}
					}

					//box->getExtParam()->set(1.0f);

					doreconnect = true;
				}

				break;
			}
		}

		if (doreconnect)
		{
			reconnect();
			_togglingChannels = false;

			return;
		}

		_togglingChannels = false;
	}

	if (control->getExtParam() != nullptr)
	{
		getInterface()->valueChanged(control);
		_owner->instrumentWasModified(getPatch(getPatch(0)->SelectedInstrument)->InstrumentDef.patchIndex);
		return;
	}

	if (control->getTag() == 99997)
	{
		GennyPatch* insPatch = getPatch(getPatch(0)->SelectedInstrument);
		insPatch->InstrumentDef.glide = (int)control->getValue();

		return;
	}
	else if (control->getTag() == 99998)
	{
		GennyPatch* insPatch = getPatch(getPatch(0)->SelectedInstrument);
		insPatch->InstrumentDef.soloMode = control->getValue() >= 0.5f;

		return;
	}
	else if (control->getTag() == 99999)
	{
		GennyPatch* insPatch = getPatch(getPatch(0)->SelectedInstrument);
		insPatch->InstrumentDef.legatoMode = control->getValue() >= 0.5f;

		return;
	}
	else if (control == _pingPongButton)
	{
		if (control->getValue() > 0.5f)
		{
			VSTBase* b = getVst()->getBase();

			Win32Frame* winFrame = (Win32Frame*)_interface->getFrame()->getPlatformFrame();
			CPoint mousePos, globalPos;
			winFrame->getCurrentMousePosition(mousePos);
			winFrame->getGlobalPosition(globalPos);
			mousePos = mousePos + globalPos;

			if (ContextMenu == nullptr)
				ContextMenu = CreatePopupMenu();

			int count = GetMenuItemCount((HMENU)ContextMenu);
			while (count > 0) {
				DeleteMenu((HMENU)ContextMenu, count - 1, MF_BYPOSITION);
				count--;
			}

			unsigned int flags = MF_STRING;
			for (int i = 0; i < kNumPingPongSettings; i++)
			{
				AppendMenuA((HMENU)ContextMenu, flags, i + 1, GennyExtParam::kDefaultPingPongSettings[i]);
			}

			BOOL r = TrackPopupMenu((HMENU)ContextMenu, TPM_RETURNCMD | TPM_RIGHTBUTTON, mousePos.x, mousePos.y, 0, winFrame->getPlatformWindow(), NULL);
			int listIndex = r - 1;
			std::string setting = "";
			if (listIndex >= 0)
			{
				if (r == kNumPingPongSettings)
				{
					LPWSTR str = MAKEINTRESOURCEW(IDD_DIALOG3);

					MEMORY_BASIC_INFORMATION mbi;
					static int dummyVariable;
					VirtualQuery(&dummyVariable, &mbi, sizeof(mbi));
					HMODULE hMod = (HMODULE)mbi.AllocationBase;
					if (DialogBoxW(hMod, str, winFrame->getPlatformWindow(), (DLGPROC)TypePingPongProc) == IDOK)
					{
						setting = pingpongEditString;
					}
				}
				else if (listIndex != 0)
					setting = GennyExtParam::kDefaultPingPongSettings[listIndex];

				GennyPatch* insPatch = getPatch(getPatch(0)->SelectedInstrument);
				std::vector<PingPongPan>& pans = insPatch->InstrumentDef.PingPongPanning;
				pans.clear();

				insPatch->InstrumentDef.parsePanString(setting);
				_pingpongLabel->setText(insPatch->InstrumentDef.PingPongString.c_str());
			}

		}
	}
	else if (control->getTag() == 9999)
	{
		if (control == _transTabButton)
		{
			_rangeSliderLow->setVisible(true);
			_rangeSliderHigh->setVisible(true);
			_rangeLabel->setVisible(true);
			_octaveSelector->setVisible(true);
			_transposeSelector->setVisible(true);
			_channelSelector->setVisible(true);
			_detuneSlider->setVisible(true);

			_soloCheckbox->setVisible(false);
			_glideSlider->setVisible(false);
			_legatoCheckbox->setVisible(false);
			_pingpongLabel->setVisible(false);
			_pingPongButton->setVisible(false);

			_delaySlider->setVisible(false);

			_transTab->setFrame(0);
		}
		else if(control == _moreTabButton)
		{
			_rangeSliderLow->setVisible(false);
			_rangeSliderHigh->setVisible(false);
			_rangeLabel->setVisible(false);
			_octaveSelector->setVisible(false);
			_transposeSelector->setVisible(false);
			_channelSelector->setVisible(false);
			_detuneSlider->setVisible(false);

			_soloCheckbox->setVisible(true);
			_glideSlider->setVisible(true);
			_legatoCheckbox->setVisible(true);
			_pingpongLabel->setVisible(true);
			_pingPongButton->setVisible(true);

			_delaySlider->setVisible(false);


			_transTab->setFrame(1); 
		}
		else
		{
			_rangeSliderLow->setVisible(false);
			_rangeSliderHigh->setVisible(false);
			_rangeLabel->setVisible(false);
			_octaveSelector->setVisible(false);
			_transposeSelector->setVisible(false);
			_channelSelector->setVisible(false);
			_detuneSlider->setVisible(false);

			_soloCheckbox->setVisible(false);
			_glideSlider->setVisible(false);
			_legatoCheckbox->setVisible(false);
			_pingpongLabel->setVisible(false);
			_pingPongButton->setVisible(false);

			_delaySlider->setVisible(true);


			_transTab->setFrame(2);
		}



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
			for(int i = 0; i < kMaxInstruments; i++)
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
			getVst()->rejiggerInstruments(false);
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
			for(int i = 0; i < kMaxInstruments; i++)
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

			getVst()->rejiggerInstruments(false);
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
			for(int i = 0; i < kMaxInstruments; i++)
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
			getVst()->rejiggerInstruments(false);
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
		for(int i = 0; i < kMaxInstruments; i++)
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

		_channels[i]->setExtParam(selectedPatch->getExt(GEParam::ChEnable, i));
		if(fmEn == 1 && _patch != nullptr && _patch->InstrumentDef.Type != GIType::DAC && _patch->InstrumentDef.Ch3Special == false)
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
	for(int i = 0; i < 5; i++)
	{
		if(i < 4)
			_channels[i + 6]->setValue(selectedPatch->InstrumentDef.Channels[i + 6]);
		else
			_channels[i + 6]->setValue(selectedPatch->InstrumentDef.snMelodicEnable);

		_channels[i + 6]->setTag(kChannelEnableStart + i + 6);

		if(i == 4)
			_channels[i + 6]->setExtParam(selectedPatch->getExt(GEParam::SnMelodicEnable));
		else
			_channels[i + 6]->setExtParam(selectedPatch->getExt(GEParam::ChEnable, i + 6));

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
	_glideSlider->reconnect();
	_rangeSliderLow->reconnect();
	_rangeSliderHigh->reconnect();
	_detuneSlider->reconnect();


	//_soloCheckbox->setValue(selectedPatch->InstrumentDef.soloMode ? 1.0f : 0.0f);
	_soloCheckbox->setExtParam(selectedPatch->getExt(GEParam::InsSoloMode));
	_legatoCheckbox->setExtParam(selectedPatch->getExt(GEParam::InsSoloLegato));


	_lChannelButton->setExtParam(selectedPatch->getExt(GEParam::InsEnableL));
	_rChannelButton->setExtParam(selectedPatch->getExt(GEParam::InsEnableR));

	//_legatoCheckbox->setValue(selectedPatch->InstrumentDef.legatoMode ? 1.0f : 0.0f);

	_rangeSliderLow->setExtParam(selectedPatch->getExt(GEParam::InsRangeHigh));
	_rangeSliderHigh->setExtParam(selectedPatch->getExt(GEParam::InsRangeLow));

	char highBuf[4];
	itoa((int)(int)_rangeSliderLow->getValue(), highBuf, 10);
	char lowBuf[4];
	itoa((int)(int)_rangeSliderHigh->getValue(), lowBuf, 10);
	_rangeLabel->setText((std::string(highBuf) + "-" + std::string(lowBuf)).c_str());


	_pingpongLabel->setText(selectedPatch->InstrumentDef.PingPongString.c_str());


	if(_patch != nullptr)
	{
		//Jumbo Channel 6 Button
		_channel6->setValue(selectedPatch->InstrumentDef.Channels[5]);
		_channel6->setTag(kChannelEnableStart + 5);

		if (_patch->InstrumentDef.Type == GIType::DAC)
		{
			_channel6->setExtParam(selectedPatch->getExt(GEParam::ChEnable, 5));
			_channel6->setVisible(true);
		}
		else
		{
			_channel6->setExtParam(nullptr);
			_channel6->setVisible(false);
		}
		_channel6->invalid();


		_channelSp3->setValue(selectedPatch->InstrumentDef.Channels[2]);
		_channelSp3->setTag(kChannelEnableStart + 2);
		if (_patch->InstrumentDef.Ch3Special)
		{
			_channelSp3->setExtParam(selectedPatch->getExt(GEParam::ChEnable, 2));
			_channelSp3->setVisible(true);
		}
		else
		{
			_channelSp3->setExtParam(nullptr);
			_channelSp3->setVisible(false);
		}
		_channelSp3->invalid();		
	}




}