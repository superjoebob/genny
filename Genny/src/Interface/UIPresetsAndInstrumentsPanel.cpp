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
UIPresetsAndInstrumentsPanel::UIPresetsAndInstrumentsPanel(const CRect& size, GennyInterface* owner):
	CView(size),
	GennyInterfaceObject(owner),
	_owner(owner),
	_tab(0),
	_presetsTabButton(nullptr),
	_importTabButton(nullptr)
{
	CFrame* frame = owner->getFrame();
	IndexBaron* baron = getIndexBaron();

#ifdef BUILD_VST
	_presetTab = new UIImage(CRect(636 + 82, 96, 636 + 82 + 206, 96 + 186), IDB_PNG39, false);
#else
	_presetTab = new UIImage(CRect(636 + 82, 96, 636 + 82 + 206, 96 + 186), PNG_PRESETSTAB, false);
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
	if(tab == kPresetsTab)
	{
		_presetTab->setFrame(0);

		_presetsView->setVisible(true);
		_importView->setVisible(false);
		_megaMidiView->setVisible(false);
	}	
	else if (tab == kImportTab)
	{
		_presetTab->setFrame(1);

		_presetsView->setVisible(false);
		_importView->setVisible(true);

		_importView->reconnect();
		_megaMidiView->setVisible(false);
	}
	else if (tab == kMegaMidiTab)
	{
		_presetTab->setFrame(2);

		_presetsView->setVisible(false);
		_importView->setVisible(false);

		_megaMidiView->setVisible(true);
		_megaMidiView->reconnect();
	}
	_tab = tab;
}

void UIPresetsAndInstrumentsPanel::addConfirmDialog() 
{ 
	_presetsView->addConfirmDialog(); 
}
