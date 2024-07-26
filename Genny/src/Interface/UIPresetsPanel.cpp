#include "UIPresetsPanel.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIPresetsAndInstrumentsPanel.h"
#include "UIPresetElement.h"

const int kConfirmOKButton = 593594;
const int kConfirmCancelButton = 593595;
static int numPresets = 10;
UIPresetsPanel::UIPresetsPanel(const CRect& size, UIPresetsAndInstrumentsPanel* owner) :
	CControl(size, owner),
	GennyInterfaceObject(owner),
	_owner(owner),
	_selection(0),
	_topItem(0),
	_initialized(false),
	_loggingMode(false),
	_category(""),
	_resetPatch(nullptr)
{
	CFrame* frame = getInterface()->getFrame();
	frame->addView(this);
	setTag(kPresetControlIndex);
	setMax(1000.0f);

	IndexBaron* baron = getIndexBaron();

	int index = getIndexBaron()->getPatchParamIndex(GPP_SelectedInstrument);
	int selection = (int)getPatch(0)->getFromBaron(baron->getIndex(index));

	for(int i = 1; i < numPresets + 1; i++)
	{
		int top = (int)size.top + ((i - 1) * 18);
		UIPresetElement* element = new UIPresetElement(CPoint(size.left, top), this, nullptr, i);
		frame->addView(element);
		_elements.push_back(element);
	} 

	setDirty(true);

	_scrollBar = new CSlider(CRect(822 + 88, 134, 822 + 88 + 14, 134 + 150), this, 999999, 134, 134 + 130, UIBitmap(PNG_SCROLLHANDLE), NULL, CPoint(), CSliderBase::Style::kTop | CSliderBase::Style::kVertical);
	frame->addView(_scrollBar);

	_upArrow = new CKickButton(CRect(824 + 88, 118, 824 + 88 + 10, 118 + 16), this, 9999999, 16, UIBitmap(PNG_UPBUTTON));
	frame->addView(_upArrow);

	_downArrow = new CKickButton(CRect(824 + 88, 264 + 20, 824 + 88 + 10, 264 + 16 + 20), this, 99999999, 16, UIBitmap(PNG_DOWNBUTTON));
	frame->addView(_downArrow);

	reconnect();
}

void UIPresetsPanel::addConfirmDialog()
{
	CFrame* frame = _owner->getFrame();

	UIImage* image = new UIImage(CRect(416, 96, 416 + 508, 96 + 206), PNG_COPYDIALOG);
	image->setVisible(false);
	frame->addView(image);
	_confirmDialog.push_back(image);
	 
	image = new UIImage(CRect(416, 96, 416 + 508, 96 + 206), PNG_LOGGINGDIALOG);
	image->setVisible(false);
	frame->addView(image);
	_confirmDialog.push_back(image);	
	
	image = new UIImage(CRect(416, 96, 416 + 508, 96 + 206), IDB_PNG55);
	image->setVisible(false);
	frame->addView(image);
	_confirmDialog.push_back(image);

	CKickButton* confirmOKButton = new CKickButton(CRect(416 + 146, 96 + 137, 416 + 146 + 102, 96 + 137 + 34), this, kConfirmOKButton, UIBitmap(PNG_COPYOKBUTTON));
	confirmOKButton->setVisible(false);
	frame->addView(confirmOKButton);
	_confirmDialog.push_back(confirmOKButton);

	CKickButton* confirmCancelButton = new CKickButton(CRect(416 + 259, 96 + 137, 416 + 259 + 102, 96 + 137 + 34), this, kConfirmCancelButton, UIBitmap(PNG_COPYCANCELBUTTON));
	confirmCancelButton->setVisible(false);
	frame->addView(confirmCancelButton);
	_confirmDialog.push_back(confirmCancelButton);
}

UIPresetsPanel::~UIPresetsPanel(void)
{
	if(_copyData.data != nullptr)
		delete _copyData.data;
}

void UIPresetsPanel::setDirty(bool dirty)
{
	CView::setDirty(dirty);
	//_owner->setDirty(dirty);
}

void UIPresetsPanel::valueChanged (CControl* control)
{
	bool force = false;
	if(control->getTag() == kConfirmOKButton)
	{
		if(control->getValue() > 0.5f)
		{
			for(int i = 0; i < _confirmDialog.size(); i++)
			{
				_confirmDialog[i]->setVisible(false);
				_confirmDialog[i]->invalid();
			}

			if(_loggingMode)
			{
				getInterface()->openLogExport();
				return;
			}
			else if (_resetPatch != nullptr)
			{
				int idx = getVst()->getPatchIndex(_resetPatch);
				if (idx > 0 && idx < getVst()->_originalPresets.size())
				{
					GennyLoaders::loadGEN(_resetPatch, &getVst()->_originalPresets[idx]);
					IndexBaron* baron = getIndexBaron();
					int count = GennyPatch::getNumParameters();
					getCurrentPatch()->Name = _copyName;

					for (int i = 0; i < 6; i++)
					{
						if (getVst()->getCore()->getChannelPatch(i) == getVst()->getCurrentPatch())
							getVst()->getCore()->clearChannelPatch(i);
					}

					getVst()->rejiggerInstruments(true);
					getInterface()->reconnect();
				}

				_resetPatch = nullptr;
				return;
			}

			if(_copyData.data != nullptr)
			{
				_copyData.dataPos = 0;
				GennyLoaders::loadGEN(getCurrentPatch(), &_copyData);
				IndexBaron* baron = getIndexBaron();
				int count = GennyPatch::getNumParameters();
				getCurrentPatch()->Name = _copyName;

				for (int i = 0; i < 6; i++)
				{
					if (getVst()->getCore()->getChannelPatch(i) == getVst()->getCurrentPatch())
						getVst()->getCore()->clearChannelPatch(i);
				}

				getVst()->rejiggerInstruments(true); 
				getInterface()->reconnect();
			}
		}
	}
	else if(control->getTag() == kConfirmCancelButton)
	{
		if(control->getValue() > 0.5f)
		{
			for(int i = 0; i < _confirmDialog.size(); i++)
			{
				_confirmDialog[i]->setVisible(false);
				_confirmDialog[i]->invalid();
			}

			if(_loggingMode)
			{
				_loggingMode = false;
				return;
			}
			if (_resetPatch != nullptr)
			{
				_resetPatch = nullptr;
				return;
			}
		}
	}
	else if(control->getTag() == 9999999)
	{
		if(_topItem > 0 && control->getValue() > 0.5f)
		{
			int numScrollItems = getVst()->getNumPatches() - numPresets;
			_scrollBar->setValue((float)(_topItem - 1) / (float)numScrollItems);
			_scrollBar->valueChanged();
		}
	}
	else if(control->getTag() == 99999999)
	{
		if(control->getValue() > 0.5f)
		{	
			int numScrollItems = getVst()->getNumPatches() - numPresets;
			if(_topItem < numScrollItems)
			{
				_scrollBar->setValue((float)(_topItem + 1) / (float)numScrollItems);
				_scrollBar->valueChanged();
			}
		}
	}
	else if(control->getTag() == 999999)
	{
		std::vector<VSTPatch*> patches = getVst()->getPatches();
		int numScrollItems = patches.size() - numPresets;


		int top = ((control->getValue() * numScrollItems) + 0.5f);
		if(top < 0)
			top = 0;

		if(top != _topItem)
		{
			_topItem = top;
			reorganize();
		}
	}
	else 
	{
		setSelection(control->getTag());
	}

	//_owner->valueChanged(control);
}

void UIPresetsPanel::draw (CDrawContext* pContext)
{
	//pContext->setClipRect(CView::getViewSize());
	if(_didScroll)
	{
		_didScroll = false;
		//_scrollBar->valueChanged();
	}
	for(int i = 0; i < _elements.size(); i++)
	{
		_elements[i]->draw(pContext);
	}
	CControl::setDirty(false);
}

void UIPresetsPanel::copyPatch(GennyPatch* patch)
{
	if(_copyData.data != nullptr)
		delete[] _copyData.data;

	_copyName = patch->Name;
	_copyData = GennyLoaders::saveGEN(patch);
}

void UIPresetsPanel::pastePatch()
{
	if(_copyData.data != nullptr)
	{
		for(int i = 0; i < _confirmDialog.size(); i++)
		{
			if(i == 1 || i == 2)
				_confirmDialog[i]->setVisible(false);
			else
				_confirmDialog[i]->setVisible(true);
			_confirmDialog[i]->invalid();
		}
	}
}

void UIPresetsPanel::resetPatch(GennyPatch* patch)
{
	_resetPatch = patch;
	for (int i = 0; i < _confirmDialog.size(); i++)
	{
		if (i == 0 || i == 1)
			_confirmDialog[i]->setVisible(false);
		else
			_confirmDialog[i]->setVisible(true);

		_confirmDialog[i]->invalid();
	}
}


void UIPresetsPanel::startLogging()
{
	_loggingMode = true;
	for(int i = 0; i < _confirmDialog.size(); i++)
	{
		if(i == 0 || i == 2)
			_confirmDialog[i]->setVisible(false);
		else
			_confirmDialog[i]->setVisible(true);
		_confirmDialog[i]->invalid();
	}
}

void UIPresetsPanel::reconnect()
{
	std::vector<VSTPatch*> patches = getVst()->getPatches();
	int selectedIndex = getPatch(0)->SelectedInstrument;
	int selection = getPatch(0)->Instruments[getInstrumentIndex(selectedIndex)];

	int numScrollItems = patches.size() - (numPresets + 1);
	_scrollBar->setWheelInc(1.0f / numScrollItems);

	_selection = -1;
	setSelection(selection); 
}

void UIPresetsPanel::setSelection(int index)
{
	if(_selection != index)
	{
		if(_selection != -1)
		{
			setValue(index);
			_owner->getOwner()->valueChanged(this);
			_owner->reconnectSelectedInstrument();
		}
		_selection = index;

		int numPatches = getVst()->getNumPatches();
		if(_selection > _topItem + numPresets || _selection < _topItem)
		{
			_topItem = _selection;
			if(_topItem > numPatches - numPresets)
				_topItem = numPatches - numPresets;
			//if(_topItem > )
			//if(_selection > numPatches - 9)
			//	_topItem = numPatches - 10;

			int numScrollItems = numPatches - numPresets;
			//if(_topItem <= numScrollItems + 1)
			_scrollBar->setValue((float)(_topItem) / (float)numScrollItems);
		}
		reorganize();	
	}
}

void UIPresetsPanel::reorganize()
{
	std::vector<VSTPatch*> patches = getVst()->getPatches();
	for(int i = 0; i < _elements.size(); i++)
	{
		GennyPatch* patch = NULL;
		if(_topItem + i < patches.size())
			patch = static_cast<GennyPatch*>(patches[_topItem + i]);

		_elements[i]->setPatchLink(patch);
		_elements[i]->setTag(_topItem + i);
		if(_elements[i]->getTag() == _selection)
			_elements[i]->select();
		else
			_elements[i]->unselect();
	}
}

bool UIPresetsPanel::onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons)
{
	if (_scrollBar->isVisible())
	{
		int numScrollItems = getVst()->getNumPatches() - numPresets;
		_scrollBar->setValue((float)(_topItem - ((int)distance)) / (float)numScrollItems);
		_scrollBar->valueChanged();
	}
	return true;
}

void UIPresetsPanel::setVisible(bool visible)
{
	for(int i = 0; i < _elements.size(); i++)
	{
		_elements[i]->setVisible(visible);
	}
	_scrollBar->setVisible(visible);
	_upArrow->setVisible(visible);
	_downArrow->setVisible(visible);
	__super::setVisible(visible);
	if(visible)
		reconnect();
}