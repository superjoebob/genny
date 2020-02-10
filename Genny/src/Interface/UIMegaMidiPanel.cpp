#include "UIMegaMidiPanel.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIPresetsAndInstrumentsPanel.h"
#include "UIPresetElement.h"
#include "UIPresetsPanel.h"
#include "UIMegaMidiPortSpinner.h"

const int kImporkjtBankButton = 9990;
UIMegaMidiPanel::UIMegaMidiPanel(const CRect& size, UIPresetsAndInstrumentsPanel* owner):
	CControl(size, owner),
	GennyInterfaceObject(owner),
	_owner(owner),
	_muteCheck(nullptr),
	_emuCheck(nullptr)
{

}

UIMegaMidiPanel::~UIMegaMidiPanel(void)
{

}

bool UIMegaMidiPanel::attached (CView* parent)
{
	bool returnValue = CControl::attached(parent);

	CFrame* frame = _owner->getFrame();
	frame->addView(this);
	setTag(kPresetControlIndex);
	setMax(10000.0f);

	IndexBaron* baron = getIndexBaron();
	int index = baron->getPatchParamIndex(GPP_SelectedInstrument);
	int selection = (int)getVst()->getPatch(0)->getFromBaron(baron->getIndex(index));

	setDirty(true);

#ifdef BUILD_VST
	UIBitmap enableButton(IDB_PNG37);
	CRect enableSize = CRect(0, 0, 18, 18);
	enableSize.offset(788, 258);
	_enableCheck = new CCheckBox(enableSize, this, 0, "", enableButton);
	_enableCheck->setValue(_vst->megaMidiPort > 0 ? 1.0f : 0.0f);
	frame->addView(_enableCheck);
	_views.push_back(_enableCheck);
#else
	_portSelector = new UIMegaMidiPortSpinner(CPoint(768 + 8, 256 + 2), this);
	frame->addView(_portSelector);
	_views.push_back(_portSelector);
#endif



	UIBitmap specialButton(IDB_PNG37);
	CRect specialSize = CRect(0, 0, 18, 18);
	specialSize.offset(900, 258);
	_muteCheck = new CCheckBox(specialSize, this, 0, "", specialButton);
	_muteCheck->setValue(_vst->megaMidiVSTMute ? 1.0f : 0.0f);
	frame->addView(_muteCheck);
	_views.push_back(_muteCheck);



	UIBitmap emuButton(IDB_PNG38);
	CRect emuSize = CRect(0, 0, 152, 18);
	emuSize.offset(722 + 46, 120);
	_emuCheck = new CCheckBox(emuSize, this, 0, "", emuButton);
	_emuCheck->setValue(_vst->accurateEmulationMode ? 1.0f : 0.0f);
	frame->addView(_emuCheck);
	_views.push_back(_emuCheck);



	reconnect();

	return returnValue;
}

void UIMegaMidiPanel::valueChanged (CControl* control)
{
	if (control == _muteCheck)
	{
		_vst->megaMidiVSTMute = control->getValue() > 0.5f ? true : false;
		_vst->_playingStatusChanged = true;
	}
	else if (control == _emuCheck)
	{
		_vst->accurateEmulationMode = control->getValue() > 0.5f ? true : false;
		_vst->_playingStatusChanged = true;
	}
#ifdef BUILD_VST
	else if (control == _enableCheck)
	{
		_vst->megaMidiPort = control->getValue() > 0.5f ? 1 : 0;
		_vst->_playingStatusChanged = true;
	}
#endif
}

void UIMegaMidiPanel::draw (CDrawContext* pContext)
{
	CControl::setDirty(false);
}

void UIMegaMidiPanel::reconnect()
{

	_muteCheck->setValue(_vst->megaMidiVSTMute ? 1.0f : 0.0f);
	_emuCheck->setValue(_vst->accurateEmulationMode ? 1.0f : 0.0f);

#ifdef BUILD_VST
	_enableCheck->setValue(_vst->megaMidiPort > 0 ? 1.0f : 0.0f);
#else
	_portSelector->reconnect();
#endif
	
	//if (((GennyPatch*)patches[selectedIndex])->InstrumentDef.Type == GIType::FM)
	//	_selectedInstrument->setTextInset(CPoint(0, 0));
	//else
	//	_selectedInstrument->setTextInset(CPoint(12, 0));
}


void UIMegaMidiPanel::setVisible(bool visible)
{
	for(int i = 0; i < _views.size(); i++)
		_views[i]->setVisible(visible);
}