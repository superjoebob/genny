#include "UIInstrumentElement.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIInstrumentsPanel.h"

//Instrument Enabled Checkbox Definition (Little square checkbox next to every instrument)
//======================================================================================
class UIInstrumentElementEnableBox : public CCheckBox
{
public:
	UIInstrumentElementEnableBox::UIInstrumentElementEnableBox (const CRect& size, CControlListener* listener, int32_t tag, UTF8StringPtr title, UIInstrumentsPanel* owner)
		:CCheckBox(size, listener, tag, title, UIBitmap(IDB_PNG30), 0)
		,_mouseDown(false)
		,controlModifier(false)
	{
		_owner = owner;
		_element = (UIInstrumentElement*)listener;
	}

	UIInstrumentElementEnableBox::~UIInstrumentElementEnableBox(void)
	{

	}

	bool UIInstrumentElementEnableBox::onWheel (const CPoint& where, const float& distance, const CButtonState& buttons)	
	{
		_owner->onWheel(where, distance, buttons);
		return true;
	}

	CMouseEventResult UIInstrumentElementEnableBox::onMouseDown (CPoint& where, const CButtonState& buttons)
	{
		if(buttons.getModifierState() == CButton::kControl)
			controlModifier = true;
		else
			controlModifier = false;

		return CCheckBox::onMouseDown(where, buttons);
	}
	int UIInstrumentElementEnableBox::getInstrumentIndex()
	{
		return _element->getInstrumentIndex();
	}


	CLASS_METHODS(UIInstrumentElementEnableBox, CCheckBox)

	//Used for Solo mode key combination
	bool controlModifier;

private:
	UIInstrumentsPanel* _owner;
	UIInstrumentElement* _element;
	bool _mouseDown;
};




//Constructor stuff
//======================================================================================
const int kThinElementSize = 168;
const int kWideElementSize = 180;
const int kEnabledCheckboxTag = 1337;
UIInstrumentElement::UIInstrumentElement(const CPoint& vPosition, UIInstrumentsPanel* vOwner, GennyPatch* vPatch, int vIndex) :
	CCheckBox(CRect(vPosition.x, vPosition.y, vPosition.x + kWideElementSize, vPosition.y + 16), nullptr, vIndex, "", UIBitmap(PNG_INSTRUMENTELEMENTCLOSED), 0),
	GennyInterfaceObject(vOwner),
	_owner(vOwner),
	_selected(false),
	_patch(vPatch),
	_typeDisplay(nullptr),
	_enabledCheckbox(nullptr),
	_wide(false)
{

}

UIInstrumentElement::~UIInstrumentElement(void)
{

}

bool UIInstrumentElement::attached (CView* parent)
{
	bool returnValue = CCheckBox::attached(parent);

	CFrame* frame = _owner->getFrame();
	IndexBaron* baron = getIndexBaron();

	_enabledCheckbox = new UIInstrumentElementEnableBox(CRect(size.left + 2, size.top + 2, size.left + 2 + 8, size.top + 2 + 10), this, kEnabledCheckboxTag, "", _owner);
	frame->addView(_enabledCheckbox);

	_typeDisplay = new CMovieBitmap(CRect((size.left + 2) + 10, size.top + 0, (size.left + 15) + 10, size.top + 12), this, -1, UIBitmap(IDB_PNG15));
	_typeDisplay->setMouseableArea(CRect(0, 0, 0, 0));
	frame->addView(_typeDisplay);


	_label = new CTextLabel(CRect(size.left + 4 + 12, size.top - 2, size.left + 142, size.top + 15), "");
	_label->setFont(kNormalFont);
	_label->setHoriAlign(kLeftText);
	_label->getFont()->setStyle(kBoldFace);
	_label->setFontColor(CColor(72, 119, 64, 255));
	frame->addView(_label);
	_label->setMouseableArea(CRect());
	_label->setBackColor(CColor(0, 0, 0, 0));
	_label->setFrameColor(CColor(0, 0, 0, 0));

	for(int i = 0; i < 10; i++)
	{
		_active.push_back(false);

		float extraOff = 0;
		int xOff = i;
		if(i > 5)
		{
			extraOff = 6;
			xOff -= 4;
		}

		CRect displaySize = CRect(0, 0, 2, i > 5 ? 4 : 6);
		displaySize.offset(size.left + 154 + (4 * xOff), size.top + 2 + extraOff);

		UIImage* light = new UIImage(displaySize,  i > 5 ? IDB_PNG29 : IDB_PNG28);
		_lights.push_back(light);
		_lightVals.push_back(0.0f);

		light->setFrame(0);
		frame->addView(light);
	}

	setPatchLink(_patch);

	return returnValue;
}
//======================================================================================




//Mouse stuff
//======================================================================================
CMouseEventResult UIInstrumentElement::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if(_owner->preparedToDrag() == this)
	{
		if(abs(_clickDragStart.x - where.x) + abs(_clickDragStart.y - where.y) > 8)
		{
			if(_owner->dragBegin(this))
			{
				_owner->getFrame()->clearMouseDown();

				CMouseEventResult res = CCheckBox::onMouseMoved(where, buttons);
				onMouseUp(CPoint(-999, -999), buttons);
				setDirty(true);
				invalid();
				return res;
			}
		}
	}

	_owner->dragUpdate(this);

	return CCheckBox::onMouseMoved(where, buttons);
}

CMouseEventResult UIInstrumentElement::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if(buttons.isRightButton())
	{
#if !BUILD_VST	
		if(_owner->getInterface() != nullptr)
		{
			VSTBase* b = _owner->getInterface()->getVst()->getBase();
			int col = -1;
			char* result = new char[255];
			memset(result, 0, 255);
			if(b->PlugHost->PromptEdit(-1, -1, "Rename Preset", result, col)) 
			{
				int idx = _owner->getInstrumentIndex(getTag());
				GennyPatch* patch = _owner->getPatch(_owner->getPatch(0)->Instruments[idx]);
				patch->Name = result;

				_owner->getOwner()->reconnect();
			}
		}
#endif	
	}
	else if(buttons.isLeftButton())
	{
		_owner->prepareToDrag(this);
		_clickDragStart = where;
		
		return CCheckBox::onMouseDown(where, buttons);
	}
	else
		return CCheckBox::onMouseDown(where, buttons);	
}

bool UIInstrumentElement::onWheel (const CPoint& where, const float& distance, const CButtonState& buttons)	
{
	_owner->onWheel(where, distance, buttons);
	return true;
}
//======================================================================================

void UIInstrumentElement::update()
{
	for(int i = 0; i < 10; i++)
	{
		if(_lightVals[i] < 0)
		{
			_lights[i]->setFrame(3);
		}
		else
		{
			if(_active[i])
				_lightVals[i] = 1.0f;
			else
			{
				if(_lightVals[i] > 0.0f)
					_lightVals[i] -= 0.5f;
				else
					_lightVals[i] = 0.0f;
			}

			_lights[i]->setFrame((1.0f - _lightVals[i]) * 2);
		}
	}
}

void UIInstrumentElement::makeChannelsDirty()
{
	for(int i = 0; i < _lights.size(); i++)
	{
		GennyPatch* channelPatch = getInterface()->getVst()->getCore()->getInstrumentPatch(i);
		GennyPatch* patchzero = getPatch(0);
		GennyPatch* mePatch = getPatch(getInstrumentIndex());
		if(mePatch == channelPatch && getInterface()->getChannelState(i))
			_active[i] = true;
		else
			_active[i] = false;
	}

	setPatchLink(_patch);
}


void UIInstrumentElement::valueChanged(CControl* control)
{
	if(control->getTag() == kEnabledCheckboxTag)
	{
		if(((UIInstrumentElementEnableBox*)control)->controlModifier)
		{
			_owner->setSolo(getInstrumentIndex());
		}
		else
		{
			if(control->getValue() > 0.5f)
				getPatch(2)->Instruments[getInstrumentIndex()] = -1;
			else
				getPatch(2)->Instruments[getInstrumentIndex()] = 1;
		}
	}

	invalid();
}


void UIInstrumentElement::valueChanged()
{
	if(value == 0.0f)
	{
		select();
		return;
	}

	_owner->valueChanged(this);
}

void UIInstrumentElement::select()
{	
	if(value == 0.0f)
	{
		setValue(1.0f);
		invalid();
	}
}

void UIInstrumentElement::unselect()
{
	if(value == 1.0f)
	{
		setValue(0.0f);
		invalid();
	}
}

void UIInstrumentElement::setPatchLink(GennyPatch* patch)
{	
	_patch = patch;

	_enabledCheckbox->setVisible(_patch != nullptr);
	if(_patch == nullptr)
	{
		setVisible(true);
		setVisible(false);
		invalid();
	}
	else
	{
		setVisible(false);
		_label->setText(patch->Name.c_str());
		setVisible(true);
		invalid();
	}

	GennyPatch* patchzero = getPatch(0);
	GennyPatch* mePatch = getPatch(getInstrumentIndex());
	for(int i = 0; i < _lights.size(); i++)
	{
		GennyPatch* channelPatch = getInterface()->getVst()->getCore()->getInstrumentPatch(i);
		if(mePatch == channelPatch && getInterface()->getChannelState(i))
			_active[i] = true;
		else
			_active[i] = false;

		if(mePatch != nullptr)
		{
			if(!mePatch->InstrumentDef.Channels[i])
				_lightVals[i] = -1;
			else if(_lightVals[i] < 0)
				_lightVals[i] = 0;
		}
	}
	updateEnabledStatus();
	update();
}

void UIInstrumentElement::setWide(bool vWide)
{
	if(vWide)
		size.right = size.left + kWideElementSize;
	else	
		size.right = size.left + kThinElementSize;

	//if(_wide != vWide)
	{
		//Offset instrument lights to the left when thin
		for(int i = 0; i < 10; i++)
		{
			float extraOff = 0;
			int xOff = i;
			if(i > 5)
			{
				extraOff = 6;
				xOff -= 4;
			}

			CRect displaySize = CRect(0, 0, 2, i > 5 ? 4 : 6);
			displaySize.offset(size.left + 156 + (4 * xOff) - (vWide ? 0 : (kWideElementSize - kThinElementSize)), size.top + 2 + extraOff);
			_lights[i]->setViewSize(displaySize, true);
			_lights[i]->invalid();
		}
	}

	invalid();
	_wide = vWide;
}

int UIInstrumentElement::getInstrumentIndex()
{		
	int tag = getTag();
	if(getPatch(1)->Instruments[tag] >= 0)
		return getPatch(1)->Instruments[tag];

	return tag;
}

void UIInstrumentElement::setVisible(bool visible)
{
	if(visible == isVisible())
		return;

	_label->setVisible(visible);

	//Set visibility of instrument channel status lights
	for(int i = 0; i < 10; i++)
	{
		//GennyPatch* channelPatch = getInterface()->getVst()->getCore()->getInstrumentPatch(i);
		_lights[i]->setVisible(/*channelPatch != nullptr ? (channelPatch->InstrumentDef.Channels[i] && visible) : */visible);
		_lights[i]->invalid();
	}


	invalid();
	CCheckBox::setVisible(visible);
}

void UIInstrumentElement::updateEnabledStatus()
{
	if(getPatch(2)->Instruments[getInstrumentIndex()] < 0)
		_enabledCheckbox->setValue(1.0f);
	else
		_enabledCheckbox->setValue(0.0f);
}

void UIInstrumentElement::invalid()
{
	if(_patch != nullptr)
	{
		if(_patch->InstrumentDef.Type == GIType::FM)
		{
			_label->setTextInset(CPoint(0, 0));
			_typeDisplay->setVisible(false);
		}
		else 
		{		
			_label->setTextInset(CPoint(12, 0));
			_typeDisplay->setVisible(true);
			if(_patch->InstrumentDef.Type != GIType::DAC)
				_typeDisplay->setValue(0.0f);
			else
				_typeDisplay->setValue(1.0);
		}
	}

	_typeDisplay->invalid();
	_label->invalid();
	_enabledCheckbox->invalid();
	CCheckBox::invalid();
}

