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
const int kInstrumentElementFlags = 3210593;
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
	CFrame* frame = getInterface()->getFrame();
	frame->addView(this);

	IndexBaron* baron = getIndexBaron();
	//int index = getIndexBaron()->getPatchParamIndex(GPP_SelectedInstrument);
	//owner->mapControl(this, index);
	setTag(kInstrumentControlIndex);
	setMax(1000.0f);

	_instrumentsTab = new UIImage(CRect(416, 96, 416 + 300, 96 + 206), PNG_INSTRUMENTTAB);
	_instrumentsTab->setMouseEnabled(false);
	_instrumentsTab->setMouseableArea(CRect(-999, -999, -999, -999));
	_instrumentsTab->setFrame(1);
	frame->addView(_instrumentsTab);

	//Initialize list of elements
	for(int i = 0; i < numInstruments; i++)
	{
		int top = (int)size.top + ((i * 18) - 8);
		UIInstrumentElement* element = new UIInstrumentElement(CPoint(size.left, top), this, nullptr, i + 1);
		frame->addView(element);
		element->specialFlags = kInstrumentElementFlags;
		_elements.push_back(element);
	}

	_selectedInstrument = new UIPESelectedInstrument(this, static_cast<GennyPatch*>(nullptr), 1);
	//frame->addView(_selectedInstrument);
	 
	setDirty(true);

	_scrollBar = new CSlider(CRect(614 - 24, 134, 614 - 24 + 14, 134 + 130 + 20), this, 999999, 134, 134 + 130, UIBitmap(PNG_SCROLLHANDLE), NULL, CPoint(), CSliderBase::Style::kTop | CSliderBase::Style::kVertical);
	frame->addView(_scrollBar);

	_scrollBarUp = new CKickButton(CRect(616- 24, 118, 616- 24 + 10, 118 + 16), this, 9999999, 16, UIBitmap(PNG_UPBUTTON));
	frame->addView(_scrollBarUp);

	_scrollBarDown = new CKickButton(CRect((616- 24), 264 + 20, (616- 24) + 10, 264 + 16 + 20), this, 99999999, 16, UIBitmap(PNG_DOWNBUTTON));
	frame->addView(_scrollBarDown);

	_addInstrumentButton = new CKickButton(CRect(548, 100, 548 + 14, 100 + 14), this, 9999, 14, UIBitmap(PNG_PLUSBUTTON));
	frame->addView(_addInstrumentButton);

	_removeInstrumentButton = new CKickButton(CRect(564, 100, 564 + 14, 100 + 14), this, 99999, 14, UIBitmap(PNG_MINUSBUTTON));
	frame->addView(_removeInstrumentButton);

	_letterDisplay = new UILetterDisplay(CPoint(414, 28), this, frame, 16);
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
		_currentDrag = _topItem + vBox->getDisplayIndex();
		return true;
	}

	return false;
}

void UIInstrumentsPanel::dragUpdate(UIInstrumentElement* vBox)
{
	if(_currentDrag != -1)
	{
		int newDrag = _topItem + vBox->getDisplayIndex();
		if(newDrag != _currentDrag)
		{
			int prevInstrumentIndex = _currentDrag;
			int newInstrumentIndex = newDrag;

			while (prevInstrumentIndex != newInstrumentIndex)
			{
				GennyPatch* reorderPatch = (GennyPatch*)getPatch(3);
				int oldIdx = prevInstrumentIndex;
				if (prevInstrumentIndex < newInstrumentIndex)
					prevInstrumentIndex++;
				else
					prevInstrumentIndex--;

				int swap = reorderPatch->Instruments[prevInstrumentIndex];
				reorderPatch->Instruments[prevInstrumentIndex] = reorderPatch->Instruments[oldIdx];
				reorderPatch->Instruments[oldIdx] = swap;
			}


			_currentDrag = newInstrumentIndex;
			_ignoreRescroll = true;
			reconnect();
			_ignoreRescroll = false;
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
	if (control->getExtParam() != nullptr)
	{
		_owner->valueChanged(control);
		return;
	}

	bool force = false;
	if(control == _addInstrumentButton)
	{
		if(control->getValue() > 0.5f)
		{	
			int instrumentSlotIndex = -1;
			for (instrumentSlotIndex = 0; instrumentSlotIndex < 16; instrumentSlotIndex++)
			{
				if (getPatch(0)->Instruments[instrumentSlotIndex] < 0)
					break;
			}

			if (instrumentSlotIndex == kMaxInstruments)
				return; //No remaining slots

			GennyPatch* patch = getCurrentPatch();
			GennyPatch* instrument = getPatch(instrumentSlotIndex);
			getPatch(0)->Instruments[instrumentSlotIndex] = getVst()->getPatchIndex(patch);

			for (int i = 0; i < kMaxInstruments; i++)
			{
				//Setup ordering for new patch
				if (getPatch(3)->Instruments[i] == -1)
				{
					getPatch(3)->Instruments[i] = instrumentSlotIndex;
					break;
				}
			}

			instrument->InstrumentDef.Enable = true;
			instrument->InstrumentDef.initializeInstrumentSettings();
			GIType::GIType type = patch->InstrumentDef.Type;
			for (int i = 0; i < 10; i++)
			{
				if (type == GIType::FM)
					instrument->InstrumentDef.Channels[i] = (i < 6) ? true : false;
				else if (type == GIType::DAC)
					instrument->InstrumentDef.Channels[i] = (i == 5) ? true : false;
				else if (type == GIType::SN)
					instrument->InstrumentDef.Channels[i] = (i > 5 && i != 9) ? true : false;
				else if (type == GIType::SNDRUM)
					instrument->InstrumentDef.Channels[i] = (i == 9) ? true : false;
				else if (type == GIType::SNSPECIAL)
					instrument->InstrumentDef.Channels[i] = (i == 8 || i == 9) ? true : false;
			}
			instrument->InstrumentDef.snMelodicEnable = type == GIType::SNSPECIAL;
			instrument->setInstrumentMode(type, true);

			reconnect();
			setSelectedInstrumentIndex(instrumentSlotIndex);
		}
	}
	else if(control == _removeInstrumentButton)
	{
		if(control->getValue() > 0.5f)
		{	
			int num = getVst()->getNumInstruments();
			if(num == 1)
				return;	

			bool shifting = false;
			int selectionIndex = 0;
			for (int i = 0; i < kMaxInstruments; i++)
			{
				//Reorganize patch ordering
				if ((shifting || getPatch(3)->Instruments[i] == _selection))
				{
					if (getPatch(3)->Instruments[i] == _selection)
						selectionIndex = i;

					if(i + 1 < kMaxInstruments)
						getPatch(3)->Instruments[i] = getPatch(3)->Instruments[i + 1];
					else
						getPatch(3)->Instruments[i] = -1;

					shifting = true;
				}
			}

			getPatch(0)->Instruments[_selection] = -1;

			while (selectionIndex > 0 && getPatch(3)->Instruments[selectionIndex] < 0)
			{
				selectionIndex--;
			}

			getPatch(0)->SelectedInstrument = getPatch(3)->Instruments[selectionIndex];



			//int instrumentListIndex = getPatch(1)->Instruments[_selection];
			//if (instrumentListIndex < 0)
			//	instrumentListIndex = index;

			////Clear instrument type on selected instrument, this helps keep it as GIType::NONE
			////when it's the 16th instrument.
			//getPatch(_selection)->InstrumentMode = GIType::NONE;
			//for(int i = _selection; i < 16; i++)
			//{
			//	int indexnext = 0;
			//	if(i + 1 < 16)
			//		indexnext = baron->getPatchParamIndex((GennyPatchParam)(GPP_Ins01 + i + 1));

			//	int index = baron->getPatchParamIndex((GennyPatchParam)(GPP_Ins01 + i));
			//	int patchIndex = -1;
			//	if(i + 1 < 16)
			//	{
			//		int nextPatch =	getPatch(0)->getFromBaron(baron->getIndex(indexnext));
			//		if(nextPatch > -1)
			//			patchIndex = getPatchIndex(getPatch(nextPatch));

			//		//Removing an instrument, shuffle all lower instruments up
			//		GennyPatch* p = (GennyPatch*)getPatch(i);
			//		GennyPatch* nextp = (GennyPatch*)getPatch(i+1);
			//		p->InstrumentDef.MidiChannel = nextp->InstrumentDef.MidiChannel;
			//		for(int i = 0; i < 10; i++)
			//				p->InstrumentDef.Channels[i] = nextp->InstrumentDef.Channels[i];
			//		p->InstrumentDef.Octave = nextp->InstrumentDef.Octave;
			//		p->InstrumentDef.Transpose = nextp->InstrumentDef.Transpose;
			//		p->InstrumentDef.Panning = nextp->InstrumentDef.Panning;
			//		p->InstrumentMode = nextp->InstrumentMode;
			//	}


			//	setTag(kInstrumentMappingStart + i);
			//	float val = value;
			//	value = patchIndex;
			//	_owner->getOwner()->valueChanged(this);
			//	value = val;

			//	setTag(kInstrumentControlIndex);

			//	if(patchIndex == -1)
			//		break;
			//}

			//if(_topItem > 0)
			//	_topItem -= 1;

			//if(sel == 0)
			//	sel = 1;
			//setSelectedInstrumentIndex(sel - 1);

			reconnect();	
			reconnectSelectedInstrument();
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
		GennyPatch* reorderPatch = (GennyPatch*)getPatch(3);
		int actualNumInstruments = 0;
		for (actualNumInstruments = 0; actualNumInstruments < kMaxInstruments; actualNumInstruments++)
		{
			if (reorderPatch->Instruments[actualNumInstruments] < 0)
				break;
		}

		int numScrollItems = actualNumInstruments - numInstruments;
		int top = ((control->getValue() * numScrollItems) + 0.5f);
		if(top < 0)
			top = 0;
		if(top != _topItem)
		{
			_topItem = top;

			_ignoreRescroll = true;
			reorganize();
			_ignoreRescroll = false;
		}
	}
	else if(control->specialFlags == kInstrumentElementFlags) //1337 is for instrument panel elements
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



	int selectedInstrumentIndex = 0;
	GennyPatch* reorderPatch = (GennyPatch*)getPatch(3);
	GennyPatch* zeroPatch = (GennyPatch*)getPatch(0);
	for(int i = 0; i < kMaxInstruments; i++)
	{
		int idx = reorderPatch->Instruments[i];
		if (idx >= 0)
		{
			indexes.push_back(idx);

			if (idx == _selection)
				selectedInstrumentIndex = i;
		}
	}

	int numScrollItems = indexes.size() - numInstruments;
	_scrollBar->setWheelInc(1.0f / numScrollItems);
	_addInstrumentButton->setVisible(indexes.size() != kMaxInstruments);

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

	if (_ignoreRescroll == false && (selectedInstrumentIndex > _topItem + (numInstruments - 1) || selectedInstrumentIndex < _topItem))
	{
		_topItem = selectedInstrumentIndex;
		if (_topItem > indexes.size() - numInstruments)
			_topItem = indexes.size() - numInstruments;

		int numScrollItems = indexes.size() - numInstruments;
		_scrollBar->setValue((float)(_topItem) / (float)numScrollItems);
	}

	if(indexes.size() > 0)
	{
		for(int i = 0; i < _elements.size(); i++)
		{
			_elements[i]->setWide(!(indexes.size() > numInstruments));

			if(_topItem + i < indexes.size())
				_elements[i]->setInstrumentIndex(indexes[_topItem + i]);
			else
				_elements[i]->setInstrumentIndex(-1);

			if(_elements[i]->getTag() == _selection)
			{
				_elements[i]->select();
				GennyPatch* presetLink = _elements[i]->getPresetLink();
				if (presetLink != nullptr)
				{
					_selectedInstrument->setPatchLink(presetLink);
					_letterDisplay->setText(presetLink->Name);
				}
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


void UIInstrumentsPanel::instrumentWasModified(int index)
{
	for (int i = 0; i < _elements.size(); i++)
	{
		if (_elements[i]->getInstrumentIndex() == index)
		{
			_elements[i]->setInstrumentIndex(index);
			break;
		}
	}
}

void UIInstrumentsPanel::reconnect()
{
	_selection = -1;
	setSelectedInstrumentIndex(getPatch(0)->SelectedInstrument);

	_selectedInstrument->reconnect();
	if(_selectedInstrument->getPatchLink() != NULL)
		_letterDisplay->setText(_selectedInstrument->getPatchLink()->Name);

	makeChannelsDirty();
}

void UIInstrumentsPanel::setSelectedInstrumentIndex(int index)
{
	//if(_selection != index)
	//{	
	//	int totalInstruments = getVst()->getNumInstruments();
	//	if(index >= 0 && index < totalInstruments)
	//	{
	//		int instrumentListIndex = getPatch(1)->Instruments[index];
	//		if (instrumentListIndex < 0)
	//			instrumentListIndex = index;

	//		int previousSelectionValue = _selection;

	//		_selection = index;
	//		setValue(index);
	//		if(previousSelectionValue != -1)
	//			_owner->valueChanged(this);

	//		if(_ignoreRescroll == false && (instrumentListIndex > _topItem + (numInstruments - 1) || instrumentListIndex < _topItem))
	//		{
	//			_topItem = instrumentListIndex;
	//			if(_topItem > totalInstruments - numInstruments)
	//				_topItem = totalInstruments - numInstruments;

	//			int numScrollItems = totalInstruments - numInstruments;
	//			_scrollBar->setValue((float)(_topItem) / (float)numScrollItems);
	//		}

	//		reorganize();
	//	}
	//}

	if (_selection != index)
	{
		int previousSelectionValue = _selection;
		_selection = index;
		setValue(index);
		if (previousSelectionValue != -1)
			_owner->valueChanged(this);

		reorganize();
	}
}

bool UIInstrumentsPanel::onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons)
{
	if (axis == CMouseWheelAxis::kMouseWheelAxisY)
	{
		int numScrollItems = getVst()->getNumInstruments() - numInstruments;
		_scrollBar->setValue((float)(_topItem - ((int)distance)) / (float)numScrollItems);
		_scrollBar->valueChanged();

		return true;
	}

	return false;
}

void UIInstrumentsPanel::setSolo(int idx)
{
	bool allOn = true;
	for(int i = 0; i < kMaxInstruments; i++)
	{
		if(i != idx && getPatch(i)->InstrumentDef.Enable)
		{
			allOn = false;
			break;
		}
	}

	for(int i = 0; i < kMaxInstruments; i++)
	{
		if(idx == i || allOn)
			getPatch(i)->InstrumentDef.Enable = true;
		else
			getPatch(i)->InstrumentDef.Enable = false;
	}
		
	for(int i = 0; i < _elements.size(); i++)
	{
		_elements[i]->updateEnabledStatus();
	}
}
