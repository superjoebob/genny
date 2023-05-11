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
	UIInstrumentElementEnableBox::UIInstrumentElementEnableBox (const CRect& size, IControlListener* listener, int32_t tag, UTF8StringPtr title, UIInstrumentsPanel* owner)
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

	bool UIInstrumentElementEnableBox::onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons)
	{
		_owner->onWheel(where, axis, distance, buttons);
		return true;
	}


	CMouseEventResult UIInstrumentElementEnableBox::onMouseUp(CPoint& where, const CButtonState& buttons)
	{
		if (buttons.isRightButton())
		{
			_owner->onMouseUpContext(tag);
			return kMouseEventHandled;
		}
		return __super::onMouseUp(where, buttons);
	}

	CMouseEventResult UIInstrumentElementEnableBox::onMouseMoved(CPoint& where, const CButtonState& buttons)
	{
		if (buttons.isRightButton())
			return kMouseEventHandled;

		return __super::onMouseMoved(where, buttons);
	}


	CMouseEventResult UIInstrumentElementEnableBox::onMouseEntered(CPoint& where, const CButtonState& buttons)
	{
		_owner->getInterface()->hoverControl(this);
		return __super::onMouseEntered(where, buttons);
	}

	CMouseEventResult UIInstrumentElementEnableBox::onMouseExited(CPoint& where, const CButtonState& buttons)
	{
		_owner->getInterface()->unhoverControl(this);
		return __super::onMouseExited(where, buttons);
	}

	CMouseEventResult UIInstrumentElementEnableBox::onMouseDown (CPoint& where, const CButtonState& buttons)
	{
		if(buttons.getModifierState() == CButton::kControl)
			controlModifier = true;
		else
			controlModifier = false;

		if (buttons.isRightButton())
			return kMouseEventHandled;
		else
			return __super::onMouseDown(where, buttons);
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
	_instrumentIndex(vIndex - 1),
	_typeDisplay(nullptr),
	_enabledCheckbox(nullptr),
	_wide(false),
	_displayIndex(vIndex - 1)
{

}

UIInstrumentElement::~UIInstrumentElement(void)
{

}

bool UIInstrumentElement::attached (CView* parent)
{
	CRect size = getViewSize();

	bool returnValue = CCheckBox::attached(parent);

	CFrame* frame = _owner->getFrame();
	IndexBaron* baron = getIndexBaron();

	_enabledCheckbox = new UIInstrumentElementEnableBox(CRect(size.left + 2, size.top + 2, size.left + 2 + 10, size.top + 2 + 12), this, kEnabledCheckboxTag, "", _owner);
	frame->addView(_enabledCheckbox);

	_typeDisplay = new UIImage(CRect((size.left + 4) + 10, size.top + 2, (size.left + 17) + 10, size.top + 14), IDB_PNG15);
	_typeDisplay->setMouseableArea(CRect(0, 0, 0, 0));
	_typeDisplay->setMouseEnabled(false);
	frame->addView(_typeDisplay);


	_label = new CTextLabel(CRect(size.left + 4 + 12, size.top, size.left + 142, size.top + 15), "");
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

	setInstrumentIndex(_instrumentIndex);
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
				//_owner->getFrame()->clearMouseDown();

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
				GennyPatch* patch = getPresetLink();
				if(patch != nullptr)
					patch->Name = result;

				_owner->getOwner()->reconnect();
			}

			return CMouseEventResult::kMouseEventHandled;
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

bool UIInstrumentElement::onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons)
{
	_owner->onWheel(where, axis, distance, buttons);
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
	GennyPatch* instrumentPatch = getInstrumentLink();
	for(int i = 0; i < _lights.size(); i++)
	{
		GennyPatch* channelPatch = getInterface()->getVst()->getCore()->getInstrumentPatch(i);
		if(instrumentPatch == channelPatch && getInterface()->getChannelState(i))
			_active[i] = true;
		else
			_active[i] = false;
	}

	setInstrumentIndex(_instrumentIndex);
}


void UIInstrumentElement::valueChanged(CControl* control)
{
	if(control == _enabledCheckbox)
	{
		if (getInstrumentLink() != nullptr)
		{
			if (((UIInstrumentElementEnableBox*)control)->controlModifier)
			{
				((UIInstrumentElementEnableBox*)control)->controlModifier = false;
				_owner->setSolo(_instrumentIndex);
				((UIInstrumentElementEnableBox*)control)->controlModifier = true;
			}
			else
			{
				_owner->valueChanged(control);

				/*if (control->getValue() > 0.5f)
					getPatch(2)->Instruments[_instrumentIndex] = -1;
				else
					getPatch(2)->Instruments[_instrumentIndex] = 1;*/
			}
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

GennyPatch* UIInstrumentElement::getPresetLink()
{
	GennyPatch* presetLink = nullptr;
	if (_instrumentIndex >= 0 && _instrumentIndex < kMaxInstruments && getPatch(0)->Instruments[_instrumentIndex] >= 0)
		presetLink = getPatch(getPatch(0)->Instruments[_instrumentIndex]);

	return presetLink;
}

GennyPatch* UIInstrumentElement::getInstrumentLink()
{
	GennyPatch* instrumentLink = nullptr;
	if (_instrumentIndex >= 0 && _instrumentIndex < kMaxInstruments && getPatch(0)->Instruments[_instrumentIndex] >= 0)
		instrumentLink = getPatch(_instrumentIndex);

	return instrumentLink;
}

void UIInstrumentElement::setInstrumentIndex(int pIdx)
{
	_instrumentIndex = pIdx;
	setTag(_instrumentIndex);

	GennyPatch* presetLink = getPresetLink();
	GennyPatch* instrumentLink = getInstrumentLink();
	_enabledCheckbox->setVisible(presetLink != nullptr);
	if(presetLink == nullptr)
	{
		setVisible(true);
		setVisible(false);
		invalid();
	}
	else
	{
		setVisible(false);
		_label->setText(presetLink->Name.c_str());
		setVisible(true);
		invalid();
	}

	for(int i = 0; i < _lights.size(); i++)
	{
		GennyPatch* channelPatch = getInterface()->getVst()->getCore()->getInstrumentPatch(i);
		if(instrumentLink == channelPatch && getInterface()->getChannelState(i))
			_active[i] = true;
		else
			_active[i] = false;

		if(instrumentLink != nullptr)
		{
			if(!instrumentLink->InstrumentDef.Channels[i])
				_lightVals[i] = -1;
			else if(_lightVals[i] < 0)
				_lightVals[i] = 0;
		}
	}

	updateEnabledStatus();
	update();
}

int UIInstrumentElement::getInstrumentIndex()
{
	return _instrumentIndex;
}

void UIInstrumentElement::setWide(bool vWide)
{
	CRect size = getViewSize();
	if(vWide)
		size.right = size.left + kWideElementSize;
	else	
		size.right = size.left + kThinElementSize;
	setViewSize(size);

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
	GennyPatch* instrumentLink = getInstrumentLink();
	if (instrumentLink != nullptr)
		_enabledCheckbox->setExtParam(instrumentLink->getExt(GEParam::InsEnable));
}

void UIInstrumentElement::invalid()
{
	GennyPatch* presetLink = getPresetLink();
	if(presetLink != nullptr)
	{
		if(presetLink->InstrumentDef.Type == GIType::FM && !presetLink->InstrumentDef.Ch3Special)
		{
			_label->setTextInset(CPoint(0, 0));
			_typeDisplay->setVisible(false);
		}
		else 
		{		
			_label->setTextInset(CPoint(12, 0));
			_typeDisplay->setVisible(true);
			if (presetLink->InstrumentDef.Type == GIType::DAC)
				_typeDisplay->setFrame(1);
			else if (presetLink->InstrumentDef.Ch3Special)
				_typeDisplay->setFrame(2);
			else
				_typeDisplay->setFrame(0);
		}
	}

	_typeDisplay->invalid();
	_label->invalid();
	_enabledCheckbox->invalid();
	CCheckBox::invalid();
}

