#include "UIImportPanel.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIPresetsAndInstrumentsPanel.h"
#include "UIPresetElement.h"
#include "UIPresetsPanel.h"

const int kImportBankButton = 9990;
const int kExportBankButton = 9991;
const int kImportInsButton = 9992;
const int kExportInsButton = 9993;
const int kLogButton = 9994;
const int kTuningButton = 9995;
const int kImportStateButton = 9996;
const int kExportStateButton = 9997;
UIImportPanel::UIImportPanel(const CRect& size, UIPresetsAndInstrumentsPanel* owner):
	CControl(size, owner),
	GennyInterfaceObject(owner),
	_owner(owner),
	_selection(0),
	_topItem(0),
	_initialized(false)
{

}

UIImportPanel::~UIImportPanel(void)
{

}

bool UIImportPanel::attached (CView* parent)
{
	bool returnValue = CControl::attached(parent);

	CFrame* frame = _owner->getFrame();
	//frame->addView(this);
	setTag(kPresetControlIndex);
	setMax(1000.0f);

	_selectedInstrument = new CTextLabel(CRect(644 + 82, 118, 644 + 82 + 180, 121 + 16), "Poopy Doody Balls");
	_selectedInstrument->setFont(kNormalFont);
	_selectedInstrument->setHoriAlign(kLeftText);
	_selectedInstrument->getFont()->setStyle(kBoldFace);
	_selectedInstrument->setFontColor(CColor(72, 119, 64, 255));
	frame->addView(_selectedInstrument);
	_selectedInstrument->setMouseableArea(CRect());
	_selectedInstrument->setBackColor(CColor(0, 0, 0, 0));
	_selectedInstrument->setFrameColor(CColor(0, 0, 0, 0));
	_views.push_back(_selectedInstrument);

	IndexBaron* baron = getIndexBaron();
	int index = baron->getPatchParamIndex(GPP_SelectedInstrument);
	int selection = (int)getVst()->getPatch(0)->getFromBaron(baron->getIndex(index));


	setDirty(true);

	CPoint buttonPos = CPoint(640 + 82, 140);
	CKickButton* importBank = new CKickButton(CRect(buttonPos.x, buttonPos.y, buttonPos.x + 98, buttonPos.y + 20), this, kImportBankButton, 20, UIBitmap(IDB_PNG35));
	frame->addView(importBank);
	_views.push_back(importBank);

	CTextLabel* label = new CTextLabel(CRect(buttonPos.x, buttonPos.y - 2, buttonPos.x + 98, buttonPos.y + 20), "Import Bank");
	label->setFont(kNormalFont);
	label->setHoriAlign(kCenterText);
	label->getFont()->setStyle(kBoldFace);
	label->setFontColor(CColor(12, 12, 12, 255));
	frame->addView(label);
	label->setMouseableArea(CRect());
	label->setBackColor(CColor(0, 0, 0, 0));
	label->setFrameColor(CColor(0, 0, 0, 0));
	_views.push_back(label);

	buttonPos.offset(100, 0);
	CKickButton* exportBank = new CKickButton(CRect(buttonPos.x, buttonPos.y, buttonPos.x + 98, buttonPos.y + 20), this, kExportBankButton, 20, UIBitmap(IDB_PNG35));
	frame->addView(exportBank);
	_views.push_back(exportBank);

	label = new CTextLabel(CRect(buttonPos.x, buttonPos.y - 2, buttonPos.x + 98, buttonPos.y + 20), "Export Bank");
	label->setFont(kNormalFont);
	label->setHoriAlign(kCenterText);
	label->getFont()->setStyle(kBoldFace);
	label->setFontColor(CColor(12, 12, 12, 255));
	frame->addView(label);
	label->setMouseableArea(CRect());
	label->setBackColor(CColor(0, 0, 0, 0));
	label->setFrameColor(CColor(0, 0, 0, 0));
	_views.push_back(label);

	buttonPos.offset(-100, 22);


	CKickButton* importState = new CKickButton(CRect(buttonPos.x, buttonPos.y, buttonPos.x + 98, buttonPos.y + 20), this, kImportStateButton, 20, UIBitmap(IDB_PNG35));
	frame->addView(importState);
	_views.push_back(importState);

	label = new CTextLabel(CRect(buttonPos.x, buttonPos.y - 2, buttonPos.x + 98, buttonPos.y + 20), "Load State");
	label->setFont(kNormalFont);
	label->setHoriAlign(kCenterText);
	label->getFont()->setStyle(kBoldFace);
	label->setFontColor(CColor(12, 12, 12, 255));
	frame->addView(label);
	label->setMouseableArea(CRect());
	label->setBackColor(CColor(0, 0, 0, 0));
	label->setFrameColor(CColor(0, 0, 0, 0));
	_views.push_back(label);

	buttonPos.offset(100, 0); 
	CKickButton* exportState = new CKickButton(CRect(buttonPos.x, buttonPos.y, buttonPos.x + 98, buttonPos.y + 20), this, kExportStateButton, 20, UIBitmap(IDB_PNG35));
	frame->addView(exportState);
	_views.push_back(exportState);

	label = new CTextLabel(CRect(buttonPos.x, buttonPos.y - 2, buttonPos.x + 98, buttonPos.y + 20), "Save State");
	label->setFont(kNormalFont);
	label->setHoriAlign(kCenterText);
	label->getFont()->setStyle(kBoldFace);
	label->setFontColor(CColor(12, 12, 12, 255));
	frame->addView(label);
	label->setMouseableArea(CRect());
	label->setBackColor(CColor(0, 0, 0, 0));
	label->setFrameColor(CColor(0, 0, 0, 0));
	_views.push_back(label);

	buttonPos.offset(-100, 22);




	CKickButton* importIns = new CKickButton(CRect(buttonPos.x, buttonPos.y, buttonPos.x + 198, buttonPos.y + 20), this, kImportInsButton, 20, UIBitmap(PNG_LONGBUTTON));
	frame->addView(importIns);
	_views.push_back(importIns);

	label = new CTextLabel(CRect(buttonPos.x, buttonPos.y - 2, buttonPos.x + 198, buttonPos.y + 20), "Import Instrument");
	label->setFont(kNormalFont);
	label->setHoriAlign(kCenterText);
	label->getFont()->setStyle(kBoldFace);
	label->setFontColor(CColor(12, 12, 12, 255));
	frame->addView(label);
	label->setMouseableArea(CRect());
	label->setBackColor(CColor(0, 0, 0, 0));
	label->setFrameColor(CColor(0, 0, 0, 0));
	_views.push_back(label);

	buttonPos.offset(0, 22);
	CKickButton* exportIns = new CKickButton(CRect(buttonPos.x, buttonPos.y, buttonPos.x + 198, buttonPos.y + 20), this, kExportInsButton, 20, UIBitmap(PNG_LONGBUTTON));

	frame->addView(exportIns);
	_views.push_back(exportIns);

	label = new CTextLabel(CRect(buttonPos.x, buttonPos.y - 2, buttonPos.x + 198, buttonPos.y + 20), "Export Instrument");
	label->setFont(kNormalFont);
	label->setHoriAlign(kCenterText);
	label->getFont()->setStyle(kBoldFace);
	label->setFontColor(CColor(12, 12, 12, 255));
	frame->addView(label);
	label->setMouseableArea(CRect());
	label->setBackColor(CColor(0, 0, 0, 0));
	label->setFrameColor(CColor(0, 0, 0, 0));
	_views.push_back(label);



	buttonPos.offset(0, 22);
	CKickButton* logButton = new CKickButton(CRect(buttonPos.x, buttonPos.y, buttonPos.x + 198, buttonPos.y + 20), this, kLogButton, 20, UIBitmap(PNG_LONGBUTTON));

	frame->addView(logButton);
	_views.push_back(logButton);

	_loggingButton = new CTextLabel(CRect(buttonPos.x, buttonPos.y - 2, buttonPos.x + 198, buttonPos.y + 20), "Log VGM");
	_loggingButton->setFont(kNormalFont);
	_loggingButton->setHoriAlign(kCenterText);
	_loggingButton->getFont()->setStyle(kBoldFace);
	_loggingButton->setFontColor(CColor(12, 12, 12, 255));
	frame->addView(_loggingButton);
	_loggingButton->setMouseableArea(CRect());
	_loggingButton->setBackColor(CColor(0, 0, 0, 0));
	_loggingButton->setFrameColor(CColor(0, 0, 0, 0));
	_views.push_back(_loggingButton);



	buttonPos.offset(0, 22);
	CKickButton* tuningButton = new CKickButton(CRect(buttonPos.x, buttonPos.y, buttonPos.x + 198, buttonPos.y + 20), this, kTuningButton, 20, UIBitmap(PNG_LONGBUTTON));

	frame->addView(tuningButton);
	_views.push_back(tuningButton);

	_tuningLabel = new CTextLabel(CRect(buttonPos.x, buttonPos.y - 2, buttonPos.x + 198, buttonPos.y + 20), "Import Tuning");
	_tuningLabel->setFont(kNormalFont);
	_tuningLabel->setHoriAlign(kCenterText);
	_tuningLabel->getFont()->setStyle(kBoldFace);
	_tuningLabel->setFontColor(CColor(12, 12, 12, 255));
	frame->addView(_tuningLabel);
	_tuningLabel->setMouseableArea(CRect());
	_tuningLabel->setBackColor(CColor(0, 0, 0, 0));
	_tuningLabel->setFrameColor(CColor(0, 0, 0, 0));
	_views.push_back(_tuningLabel);

	reconnect();

	return returnValue;
}

void UIImportPanel::setDirty(bool dirty)
{
	CControl::setDirty(dirty);
	//_owner->setDirty(dirty);
}

void UIImportPanel::valueChanged (CControl* control)
{
	if(control->getTag() == kImportInsButton && control->getValue() > 0.5f)
	{
		getInterface()->openInstrumentImport();
	}
	else if(control->getTag() == kExportInsButton && control->getValue() > 0.5f)
	{
		getInterface()->openInstrumentExport();
	}
	else if(control->getTag() == kImportBankButton && control->getValue() > 0.5f)
	{
		getInterface()->openBankImport();
	}
	else if(control->getTag() == kExportBankButton && control->getValue() > 0.5f)
	{
		getInterface()->openBankExport();
	}
	else if (control->getTag() == kImportStateButton && control->getValue() > 0.5f)
	{
		getInterface()->openStateImport();
	}
	else if (control->getTag() == kExportStateButton && control->getValue() > 0.5f)
	{
		getInterface()->openStateExport();
	}
	else if(control->getTag() == kLogButton && control->getValue() > 0.5f)
	{
		if(getInterface()->getLogging() == false)
			_owner->getPresetsView()->startLogging();
		else
			getVst()->stopLogging();
	}	
	else if(control->getTag() == kTuningButton && control->getValue() > 0.5f)
	{
		if(getVst()->getFrequencyTable() != getVst()->getDefaultFrequencyTable())
		{
			getVst()->setFrequencyTable(nullptr);

			_tuningLabel->setText("Import Tuning");
			_tuningLabel->invalid();
		}
		else
		{
			getInterface()->openTuningImport();
		}
	}
}

void UIImportPanel::draw (CDrawContext* pContext)
{
	CControl::setDirty(false);
}

void UIImportPanel::reconnect()
{
	IndexBaron* baron = getIndexBaron();
	int index = baron->getPatchParamIndex(GPP_SelectedInstrument);
	std::vector<VSTPatch*> patches = getVst()->getPatches();

	GennyPatch* patch0 = static_cast<GennyPatch*>(getVst()->getPatch(0));
	
	int selectedIndex = (int)patch0->Instruments[patch0->SelectedInstrument];
	_selectedInstrument->setText(patches[selectedIndex]->Name.c_str());

	if(getInterface()->getLogging())
	{
		_loggingButton->setText("Stop Logging");
	}
	else
	{
		_loggingButton->setText("Log VGM");
	}

	if(getVst()->getFrequencyTable() == getVst()->getDefaultFrequencyTable())
	{
		_tuningLabel->setText("Import Tuning");
		_tuningLabel->invalid();
	}
	else
	{
		_tuningLabel->setText("Reset Tuning");
		_tuningLabel->invalid();
	}


	//if (((GennyPatch*)patches[selectedIndex])->InstrumentDef.Type == GIType::FM)
	//	_selectedInstrument->setTextInset(CPoint(0, 0));
	//else
	//	_selectedInstrument->setTextInset(CPoint(12, 0));
}


void UIImportPanel::setVisible(bool visible)
{
	for(int i = 0; i < _views.size(); i++)
		_views[i]->setVisible(visible);
}