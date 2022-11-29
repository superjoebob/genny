#include "UIPresetsAndInstrumentsPanel.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIInstrumentsPanel.h"
#include "UIPresetsPanel.h"
#include "UIImportPanel.h"
#include "UIMegaMidiPanel.h"

const int kPresetsTab = 0;
const int kImportTab = 1;
const int kMegaMidiTab = 2;
const int kInfoTab = 3;
UIPresetsAndInstrumentsPanel::UIPresetsAndInstrumentsPanel(const CRect& size, GennyInterface* owner):
	CView(size),
	GennyInterfaceObject(owner),
	_owner(owner),
	_tab(0),
	_presetsTabButton(nullptr),
	_importTabButton(nullptr)
{
	CFrame* frame = getInterface()->getFrame();
	IndexBaron* baron = getIndexBaron();

#ifdef BUILD_VST
	_presetTab = new UIImage(CRect(636 + 82, 96, 636 + 82 + 206, 96 + 206), IDB_PNG39, false);
#else
	_presetTab = new UIImage(CRect(636 + 82, 96, 636 + 82 + 206, 96 + 206), PNG_PRESETSTAB, false);
#endif
	frame->addView(_presetTab);




	_presetsTabButton = new CKickButton(CRect(638 + 82, 98, 638 + 82 + 68, 98 + 20), this, 9999, 20, nullptr );
	frame->addView(_presetsTabButton);

	_importTabButton = new CKickButton(CRect(802, 96, 878, 116), this, 999, 20, nullptr );
	frame->addView(_importTabButton);

	_megamidiTabButton = new CKickButton(CRect(880, 96, 910, 116), this, 999, 20, nullptr);
	frame->addView(_megamidiTabButton);	


	_presetsView = new UIPresetsPanel(CRect(640 + 82, 120, 640 + 82 + 180, 120 + 158), this);

	_instrumentView = new UIInstrumentsPanel(CRect(420, 128, 420 + 192, 128 + 140), this);

	_importView = new UIImportPanel(CRect(0,0,0,0), this);
	frame->addView(_importView);
	_importView->setVisible(false);

	_megaMidiView = new UIMegaMidiPanel(CRect(0, 0, 0, 0), this);
	frame->addView(_megaMidiView);
	_megaMidiView->setVisible(false);

	//UIImage* velocityMapWindow = new UIImage(CRect(0, 0, 954, 552), IDB_PNG36, false);
	//frame->addView(velocityMapWindow); 

	_infoTab = new UIImage(CRect(416, 96, 416 + 508, 96 + 206), PNG_INFOTAB, false);
	_infoTab->setVisible(false);
	frame->addView(_infoTab);

	_infoTabButton = new CKickButton(CRect(910, 98, 910 + 12, 98 + 16), this, 999, 16, new CBitmap((CResourceDescription)PNG_INFOBUTTON));
	frame->addView(_infoTabButton);

	_infoLabel = new CTextLabel(CRect(422, -84, 416 + 508, 96 + 206), UTF8String(


		"GENNY v" + std::string(GENNY_VERSION_STRING) + " - CORPTRON GAMES CORP. 2022\n"
		"-----------------------------------------------------------------------------------------------------------------------\n"
		"Created by Landon Podbielski (superjoebob), but could not have been made without:\n"
		"\n"
		"Jarek Burczynski, hiro-shi(Hiromitsu Shioya) - MAME YM2612 Emulation Code\n"
		"Eke-Eke - Genesis Plus GX YM2612/SN76489 Emulation Code\n" 
		"Tiido Priimägi, Maxim - YM2612 Hardware Experts\n"
		"Aidan Lawrence - MEGA MIDI Hardware\n"
		"VSTGUI - (c) Steinberg Media Technologies, All Rights Reserved\n"
		"\n"
		"Thanks to Howard Drossin, John Baker, Tommy Tallarico, Matt Furniss,\n"
		"Masato Nakamura, Masaru Setsumaru, Yuzo Koshiro and many more for composing\n"
		"some of the great YM2612 music that GENNY's presets come from.\n"
		"GENNY is and will always be freeware.\n"
	
	));

	_infoLabel->setFont(kNormalFont); 
	_infoLabel->setHoriAlign(kLeftText);
	_infoLabel->getFont()->setStyle(kBoldFace);
	_infoLabel->setFontColor(CColor(16, 20, 16, 255));
	frame->addView(_infoLabel);
	_infoLabel->setMouseableArea(CRect());
	_infoLabel->setBackColor(CColor(0, 0, 0, 0));
	_infoLabel->setFrameColor(CColor(0, 0, 0, 0));
	_infoLabel->setVisible(false);
}

UIPresetsAndInstrumentsPanel::~UIPresetsAndInstrumentsPanel(void)
{

}


void UIPresetsAndInstrumentsPanel::updateInstrumentChannels()
{
	_instrumentView->updateInstrumentChannels();
}



void UIPresetsAndInstrumentsPanel::makeChannelsDirty()
{
	_instrumentView->makeChannelsDirty();
}


void UIPresetsAndInstrumentsPanel::valueChanged (CControl* control)
{
	if(control == _importTabButton)
		setTab(kImportTab);
	else if(control == _presetsTabButton)
		setTab(kPresetsTab);
	else if (control == _megamidiTabButton)
		setTab(kMegaMidiTab);
	else if (control == _infoTabButton)
	{
		if(control->getValue() > 0.5f)
			setTab(kInfoTab);
	}
	else
		_owner->valueChanged(control);
}

void UIPresetsAndInstrumentsPanel::draw (CDrawContext* pContext)
{
	CView::setDirty(false);
}

void UIPresetsAndInstrumentsPanel::setParam(int index, float val)
{

}

void UIPresetsAndInstrumentsPanel::reconnect()
{
	/*IndexBaron* baron = _owner->getIndexBaron();
	std::map<int, CControl*>::iterator it;
	for(it = _controls.begin(); it != _controls.end(); it++)
	{
		(*it).second->setValue(getOwner()->getOwner()->getCurrentPatch()->getFromBaron(baron->getIndex((*it).second->getTag())));
	}*/
	_instrumentView->reconnect();
	_presetsView->reconnect();
	_importView->reconnect();
	_megaMidiView->reconnect();
	setTab(_tab);
}
	
void UIPresetsAndInstrumentsPanel::reconnectSelectedInstrument()
{
	_instrumentView->reconnectSelectedInstrument();
}


void UIPresetsAndInstrumentsPanel::setTab(int tab)
{
	setDirty(true);
	_presetsView->setDirty(true);
	_importView->setDirty(true);
	_importView->setDirty(true);
	_megaMidiView->setDirty(true);
	_presetTab->setDirty(true);

	if(tab == kPresetsTab)
	{
		_presetTab->setFrame(0);

		_presetsView->setVisible(true);
		_importView->setVisible(false);
		_megaMidiView->setVisible(false);
		_infoTab->setVisible(false);
		_infoLabel->setVisible(false);
	}	
	else if (tab == kImportTab)
	{
		_presetTab->setFrame(1);

		_presetsView->setVisible(false);
		_importView->setVisible(true);

		_importView->reconnect();
		_megaMidiView->setVisible(false);
		_infoTab->setVisible(false);
		_infoLabel->setVisible(false);
	}
	else if (tab == kMegaMidiTab)
	{
		_presetTab->setFrame(2);

		_presetsView->setVisible(false);
		_importView->setVisible(false);

		_megaMidiView->setVisible(true);
		_megaMidiView->reconnect();
		_infoTab->setVisible(false);
		_infoLabel->setVisible(false);
	}
	else if (tab == kInfoTab)
	{
		if (_infoTab->isVisible())
			setTab(_tab);
		else
		{
			_presetsView->setVisible(false);
			_importView->setVisible(false);

			_megaMidiView->setVisible(true);
			_megaMidiView->reconnect();
			_infoTab->setVisible(true);
			_infoLabel->setVisible(true);
		}

		return;
	}

	_tab = tab;
}

void UIPresetsAndInstrumentsPanel::addConfirmDialog() 
{ 
	_presetsView->addConfirmDialog(); 
}
