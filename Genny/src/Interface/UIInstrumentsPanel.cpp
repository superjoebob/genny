#include "UIInstrumentsPanel.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIPresetsAndInstrumentsPanel.h"
#include "UIInstrumentElement.h"
#include "UIPESelectedInstrument.h"
#include "UIPresetsPanel.h"
#include "UIDigitKnob.h"

static int numInstruments = 10;
UIInstrumentsPanel::UIInstrumentsPanel(const CRect& size, UIPresetsAndInstrumentsPanel* owner):
	CControl(size, owner),
	GennyInterfaceObject(owner),
	_owner(owner),
	_selection(-1),
	_topItem(0),
	_scrollBar(nullptr),
	_addInstrumentButton(nullptr),
	_removeInstrumentButton(nullptr),
	_scrollBarUp(nullptr),
	_scrollBarDown(nullptr),
	_instrumentsTab(nullptr),
	_currentDrag(-1),
	_preparingToDrag(false),
	_ignoreRescroll(false)
{
	CFrame* frame = owner->getFrame();
	frame->addView(this);

	IndexBaron* baron = getIndexBaron();
	//int index = getIndexBaron()->getPatchParamIndex(GPP_SelectedInstrument);
	//owner->mapControl(this, index);
	setTag(kInstrumentControlIndex);
	setMax(10000.0f);

	_instrumentsTab = new UIImage(CRect(416, 96, 416 + 300, 96 + 186), PNG_INSTRUMENTTAB);
	_instrumentsTab->setFrame(1);
	frame->addView(_instrumentsTab);

	//Initialize list of elements
	for(int i = 0; i < numInstruments; i++)
	{
		int top = (int)size.top + ((i * 16) - 8);
		UIInstrumentElement* element = new UIInstrumentElement(CPoint(size.left, top), this, nullptr, i + 1);
		frame->addView(element);
		_elements.push_back(element);
	}

	_selectedInstrument = new UIPESelectedInstrument(this, static_cast<GennyPatch*>(nullptr), 1);
	//frame->addView(_selectedInstrument);
	 
	setDirty(true);

	_scrollBar = new CSlider(CRect(614 - 24, 134, 614 - 24 + 14, 134 + 130), this, 999999, 134, 134 + 130 - 20, UIBitmap(PNG_SCROLLHANDLE), NULL, CPoint(), kTop | kVertical);
	frame->addView(_scrollBar);

	_scrollBarUp = new CKickButton(CRect(616- 24, 118, 616- 24 + 10, 118 + 16), this, 9999999, 16, UIBitmap(PNG_UPBUTTON));
	frame->addView(_scrollBarUp);

	_scrollBarDown = new CKickButton(CRect(616- 24, 264, 616- 24 + 10, 264 + 16), this, 99999999, 16, UIBitmap(PNG_DOWNBUTTON));
	frame->addView(_scrollBarDown);

	_addInstrumentButton = new CKickButton(CRect(548, 100, 548 + 14, 100 + 14), this, 9999, 14, UIBitmap(PNG_PLUSBUTTON));
	frame->addView(_addInstrumentButton);

	_removeInstrumentButton = new CKickButton(CRect(564, 100, 564 + 14, 100 + 14), this, 99999, 14, UIBitmap(PNG_MINUSBUTTON));
	frame->addView(_removeInstrumentButton);

	_letterDisplay = new UILetterDisplay(CPoint(414, 28), this, 16);
	frame->addView(_letterDisplay);

	reconnect();
}

UIInstrumentsPanel::~UIInstrumentsPanel(void)
{
	delete _selectedInstrument;
}

void UIInstrumentsPanel::updateInstrumentChannels()
{
	for(int i = 0; i < numInstruments; i++)
	{
		_elements[i]->update();
	}
}

void UIInstrumentsPanel::makeChannelsDirty()
{
	for(int i = 0; i < numInstruments; i++)
	{
		_elements[i]->makeChannelsDirty();
	}
}

bool UIInstrumentsPanel::dragBegin(UIInstrumentElement* vBox)
{
	if(_currentDrag == -1)
	{
		_currentDrag = vBox->getTag();
		return true;
	}

	return false;
}

void UIInstrumentsPanel::dragUpdate(UIInstrumentElement* vBox)
{
	if(_currentDrag != -1)
	{
		if(vBox->getTag() != _currentDrag)
		{
			int prevInstrumentIndex = _currentDrag;
			int newInstrumentIndex = vBox->getTag();

			GennyPatch* reorderPatch = (GennyPatch*)getPatch(1);
			GennyPatch* prevInstrument = (GennyPatch*)getPatch(((GennyPatch*)getPatch(0))->Instruments[prevInstrumentIndex]);
			GennyPatch* newInstrument = (GennyPatch*)getPatch(((GennyPatch*)getPatch(0))->Instruments[newInstrumentIndex]);
			
			if(reorderPatch->Instruments[prevInstrumentIndex] < 0)
				reorderPatch->Instruments[prevInstrumentIndex] = prevInstrumentIndex;
			if(reorderPatch->Instruments[newInstrumentIndex] < 0)
				reorderPatch->Instruments[newInstrumentIndex] = newInstrumentIndex;

			int swap = reorderPatch->Instruments[newInstrumentIndex];
			reorderPatch->Instruments[newInstrumentIndex] = reorderPatch->Instruments[prevInstrumentIndex];
			reorderPatch->Instruments[prevInstrumentIndex] = swap;


			//int temp = ((GennyPatch*)getPatch(0))->Instruments[prevInstrumentIndex];
			//((GennyPatch*)getPatch(0))->Instruments[prevInstrumentIndex] = ((GennyPatch*)getPatch(0))->Instruments[newInstrumentIndex];
			//((GennyPatch*)getPatch(0))->Instruments[newInstrumentIndex] = temp;

			if(((GennyPatch*)getPatch(0))->SelectedInstrument == prevInstrumentIndex)
				((GennyPatch*)getPatch(0))->SelectedInstrument = newInstrumentIndex;
			else if(((GennyPatch*)getPatch(0))->SelectedInstrument == newInstrumentIndex)
				((GennyPatch*)getPatch(0))->SelectedInstrument = prevInstrumentIndex;

			_ignoreRescroll = true;
			reconnect();
			_ignoreRescroll = false;



			_currentDrag = vBox->getTag();
		}
	}
}

void UIInstrumentsPanel::dragEnd()
{
	_currentDrag = -1;
	_preparingToDrag = nullptr;
}


void UIInstrumentsPanel::setDirty(bool dirty)
{
	CView::setDirty(dirty);
	//_owner->setDirty(dirty);
}

void UIInstrumentsPanel::valueChanged (CControl* control)
{
	bool force = false;
	if(control == _addInstrumentButton)
	{
		if(control->getValue() > 0.5f)
		{	
			int instrumentIndex = getVst()->getNumInstruments();
			if(instrumentIndex == 16)
				return;

			IndexBaron* baron = getIndexBaron();
			int indexprev = baron->getPatchParamIndex((GennyPatchParam)(GPP_Ins01 + _selection));
			int prevPatch = getPatch(0)->getFromBaron(baron->getIndex(indexprev));
			int patchIndex = getVst()->getPatchIndex(getVst()->getPatch(prevPatch));

			setTag(kInstrumentMappingStart + instrumentIndex);
			float val = value;
			value = patchIndex;
			getInterface()->valueChanged(this);
			value = val;

			setTag(kInstrumentControlIndex);

			//Set initial enabled channels based on patch type
			GIType::GIType type = getPatch(patchIndex)->InstrumentDef.Type;
			for(int i = 0; i < 10; i++)
			{
				if(type == GIType::FM)
					getPatch(instrumentIndex)->setFromBaron(baron->getIndex(getIndexBaron()->getInsParamIndex((GennyInstrumentParam)(GIP_Channel0 + i))), (i < 6) ? 1.0f : 0.0f); 
				else if(type == GIType::DAC)
					getPatch(instrumentIndex)->setFromBaron(baron->getIndex(getIndexBaron()->getInsParamIndex((GennyInstrumentParam)(GIP_Channel0 + i))), (i == 5) ? 1.0f : 0.0f);
				else if(type == GIType::SN)
					getPatch(instrumentIndex)->setFromBaron(baron->getIndex(getIndexBaron()->getInsParamIndex((GennyInstrumentParam)(GIP_Channel0 + i))), (i > 5) ? 1.0f : 0.0f);
				else if(type == GIType::SNDRUM)
					getPatch(instrumentIndex)->setFromBaron(baron->getIndex(getIndexBaron()->getInsParamIndex((GennyInstrumentParam)(GIP_Channel0 + i))), (i == 9) ? 1.0f : 0.0f);
			}
			getPatch(instrumentIndex)->InstrumentMode = type;

			reconnect();
			setSelectedInstrumentIndex(instrumentIndex);
		}
	}
	else if(control == _removeInstrumentButton)
	{
		if(control->getValue() > 0.5f)
		{	
			int num = getVst()->getNumInstruments();
			if(num == 1)
				return;

			IndexBaron* baron = getIndexBaron();


			int sel = _selection;

			//Clear instrument type on selected instrument, this helps keep it as GIType::NONE
			//when it's the 16th instrument.
			getPatch(_selection)->InstrumentMode = GIType::NONE;
			for(int i = _selection; i < 16; i++)
			{
				int indexnext = 0;
				if(i + 1 < 16)
					indexnext = baron->getPatchParamIndex((GennyPatchParam)(GPP_Ins01 + i + 1));

				int index = baron->getPatchParamIndex((GennyPatchParam)(GPP_Ins01 + i));
				int patchIndex = -1;
				if(i + 1 < 16)
				{
					int nextPatch =	getPatch(0)->getFromBaron(baron->getIndex(indexnext));
					if(nextPatch > -1)
						patchIndex = getPatchIndex(getPatch(nextPatch));

					//Removing an instrument, shuffle all lower instruments up
					GennyPatch* p = (GennyPatch*)getPatch(i);
					GennyPatch* nextp = (GennyPatch*)getPatch(i+1);
					p->InstrumentDef.MidiChannel = nextp->InstrumentDef.MidiChannel;
					for(int i = 0; i < 10; i++)
							p->InstrumentDef.Channels[i] = nextp->InstrumentDef.Channels[i];
					p->InstrumentDef.Octave = nextp->InstrumentDef.Octave;
					p->InstrumentDef.Transpose = nextp->InstrumentDef.Transpose;
					p->InstrumentDef.Panning = nextp->InstrumentDef.Panning;
					p->InstrumentMode = nextp->InstrumentMode;
				}


				setTag(kInstrumentMappingStart + i);
				float val = value;
				value = patchIndex;
				_owner->getOwner()->valueChanged(this);
				value = val;

				setTag(kInstrumentControlIndex);

				if(patchIndex == -1)
					break;
			}

			if(_topItem > 0)
				_topItem -= 1;

			if(sel == 0)
				sel = 1;
			setSelectedInstrumentIndex(sel - 1);
			reconnect();		
		}
	}
	else if(control == _scrollBarUp)
	{
		if(_topItem > 0 && control->getValue() > 0.5f)
		{
			int numScrollItems =getVst()->getNumInstruments() - numInstruments;
			_scrollBar->setValue((float)(_topItem - 1) / (float)numScrollItems);
			_scrollBar->valueChanged();
		}
	}
	else if(control == _scrollBarDown)
	{
		if(control->getValue() > 0.5f)
		{	
			int numScrollItems = getVst()->getNumInstruments() - numInstruments;
			if(_topItem < numScrollItems)
			{
				_scrollBar->setValue((float)(_topItem + 1) / (float)numScrollItems);
				_scrollBar->valueChanged();
			}
		}
	}
	else if(control == _scrollBar)
	{
		std::vector<VSTPatch*> patches = getVst()->getPatches();
		int numScrollItems = getVst()->getNumInstruments() - numInstruments;

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
		setSelectedInstrumentIndex(control->getTag());
}

void UIInstrumentsPanel::setScrollBarVisible(bool val)
{
	_scrollBar->setVisible(val);
	_scrollBarUp->setVisible(val);
	_scrollBarDown->setVisible(val);
}

void UIInstrumentsPanel::setValue(float val)
{
	CControl::setValue(val);
	setSelectedInstrumentIndex((int)val);
}

void UIInstrumentsPanel::reconnectSelectedInstrument()
{
	_selectedInstrument->reconnect();
	reconnect();
}

void UIInstrumentsPanel::reorganize()
{
	IndexBaron* baron = getIndexBaron();
	std::vector<VSTPatch*> patches = getVst()->getPatches();
	std::vector<int> indexes;
	GennyPatch* reorderPatch = (GennyPatch*)getPatch(1);
	for(int i = 0; i < 16; i++)
	{
		int idx = (int)((GennyPatch*)getVst()->getPatch(0))->Instruments[i]; 
		if(reorderPatch->Instruments[i] >= 0)
			idx = (int)((GennyPatch*)getVst()->getPatch(0))->Instruments[reorderPatch->Instruments[i]]; 

		if(idx >= 0)
			indexes.push_back(idx);
	}
	int numScrollItems = indexes.size() - numInstruments;
	_scrollBar->setWheelInc(1.0f / numScrollItems);

	_addInstrumentButton->setVisible(indexes.size() != 16);

	if(indexes.size() > numInstruments)
	{
		_instrumentsTab->setFrame(0);
		setScrollBarVisible(true);
	}
	else
	{
		_instrumentsTab->setFrame(1);
		setScrollBarVisible(false);
		_topItem = 0;
	}

	if(indexes.size() > 0)
	{
		for(int i = 0; i < _elements.size(); i++)
		{
			_elements[i]->setWide(!(indexes.size() > numInstruments));

			GennyPatch* patch = NULL;
			if(_topItem + i < indexes.size())
				patch = static_cast<GennyPatch*>(patches[indexes[_topItem + i]]);
			
			_elements[i]->setTag(_topItem + i);
			_elements[i]->setPatchLink(patch);
			if(_elements[i]->getTag() == _selection)
			{
				_elements[i]->select();
				_selectedInstrument->setPatchLink(patch);
				_letterDisplay->setText(patch->Name);
			}
			else
				_elements[i]->unselect();
		}
	}

	makeChannelsDirty();
}

void UIInstrumentsPanel::draw (CDrawContext* pContext)
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

void UIInstrumentsPanel::reconnect()
{
	IndexBaron* baron = getIndexBaron();
	int index = baron->getPatchParamIndex(GPP_SelectedInstrument);

	std::vector<VSTPatch*> patches = getVst()->getPatches();
	int selection = (int)getVst()->getPatch(0)->getFromBaron(baron->getIndex(index));

	_selection = -1;
	setSelectedInstrumentIndex(selection);

	_selectedInstrument->reconnect();
	if(_selectedInstrument->getPatchLink() != NULL)
		_letterDisplay->setText(_selectedInstrument->getPatchLink()->Name);

	makeChannelsDirty();
}

void UIInstrumentsPanel::setSelectedInstrumentIndex(int index)
{
	if(_selection != index)
	{	
		int totalInstruments = getVst()->getNumInstruments();
		if(index >= 0 && index < totalInstruments)
		{
			int previousSelectionValue = _selection;

			_selection = index;
			setValue(index);
			if(previousSelectionValue != -1)
				_owner->valueChanged(this);

			if(_ignoreRescroll == false && (_selection > _topItem + (numInstruments - 1) || _selection < _topItem))
			{
				_topItem = _selection;
				if(_topItem > totalInstruments - numInstruments)
					_topItem = totalInstruments - numInstruments;

				int numScrollItems = totalInstruments - numInstruments;
				_scrollBar->setValue((float)(_topItem) / (float)numScrollItems);
			}

			reorganize();
		}
	}
}

bool UIInstrumentsPanel::onWheel (const CPoint& where, const float& distance, const CButtonState& buttons)	
{
	int numScrollItems = getVst()->getNumInstruments() - numInstruments;
	_scrollBar->setValue((float)(_topItem - ((int)distance)) / (float)numScrollItems);
	_scrollBar->valueChanged();


	return true;
}

void UIInstrumentsPanel::setSolo(int idx)
{
	bool allOn = true;
	for(int i = 0; i < 16; i++)
	{
		if(i != idx && getPatch(2)->Instruments[i] < 0)
		{
			allOn = false;
			break;
		}
	}

	for(int i = 0; i < 16; i++)
	{
		if(idx == i || allOn)
			getPatch(2)->Instruments[i] = -1;
		else
			getPatch(2)->Instruments[i] = 1;
	}
		
	for(int i = 0; i < _elements.size(); i++)
	{
		_elements[i]->updateEnabledStatus();
	}
}
