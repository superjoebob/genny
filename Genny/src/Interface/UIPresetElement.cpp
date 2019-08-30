#include "UIPresetElement.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIPresetsPanel.h"
#include "GennyLoaders.h"

//Constructor stuff
//======================================================================================
const int kCopyButton = 9990;
const int kPasteButton = 9991;
const int kElementWidth = 186;
UIPresetElement::UIPresetElement(const CPoint& vPosition, UIPresetsPanel* vOwner, GennyPatch* vPatch, int vIndex):
	CCheckBox(CRect(vPosition.x, vPosition.y, vPosition.x + kElementWidth, vPosition.y + 14), nullptr, vIndex, "", UIBitmap(PNG_PRESETELEMENT), 0),
	GennyInterfaceObject(vOwner),
	_owner(vOwner),
	_selected(false),
	_patch(vPatch),
	_copyButton(NULL),
	_typeDisplay(nullptr)
{

}

UIPresetElement::~UIPresetElement(void)
{

}

bool UIPresetElement::attached (CView* parent)
{
	bool returnValue = CCheckBox::attached(parent);
		
	CFrame* frame = _owner->getFrame();
	IndexBaron* baron = getIndexBaron();

	_typeDisplay = new CMovieBitmap(CRect(size.left + 2, size.top + 2, size.left + 15, size.top + 15), this, -1, UIBitmap(IDB_PNG15));
	_typeDisplay->setMouseableArea(CRect(0, 0, 0, 0));
	frame->addView(_typeDisplay);

	_label = new CTextLabel(CRect(size.left + 4, size.top, size.left + 180, size.top + 15), "");
	_label->setFont(kNormalFont);
	_label->setHoriAlign(kLeftText);
	_label->getFont()->setStyle(kBoldFace);
	_label->setFontColor(CColor(72, 119, 64, 255));
	_label->setMouseableArea(CRect());
	_label->setBackColor(CColor(0, 0, 0, 0));
	_label->setFrameColor(CColor(0, 0, 0, 0));
	frame->addView(_label);

	UIBitmap copyImage =  UIBitmap(PNG_COPY);
	_copyButton = new CKickButton(CRect(size.left + 158, size.top + 2, size.left + 158 + copyImage.getWidth(), size.top + 2 + (copyImage.getHeight() / 2)), this, kCopyButton, copyImage);
	_owner->getFrame()->addView(_copyButton);
	_copyButton->setVisible(false);
	
	UIBitmap pasteImage =  UIBitmap(PNG_PASTE);
	_pasteButton = new CKickButton(CRect(size.left + 172, size.top + 2, size.left + 172 + pasteImage.getWidth(), size.top + 4 + (pasteImage.getHeight() / 2)), this, kPasteButton, pasteImage);
	_owner->getFrame()->addView(_pasteButton);
	_pasteButton->setVisible(false);

	setPatchLink(_patch);

	return returnValue;
}
//======================================================================================

bool UIPresetElement::onWheel (const CPoint& where, const float& distance, const CButtonState& buttons)	
{
	_owner->onWheel(where, distance, buttons);
	return true;
}

CMouseEventResult UIPresetElement::onMouseDown (CPoint& where, const CButtonState& buttons)
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
				getPatchLink()->Name = result;
				_owner->getOwner()->reconnect();
			}


			//b->PlugHost->Dispatcher(b->HostTag, FHD_SetNewColor, 0, 5);
		}
#endif	
	}
	else
		return CCheckBox::onMouseDown(where, buttons);	
}


void UIPresetElement::valueChanged (CControl* control)
{
	if(control->getTag() == kCopyButton)
	{
		if(control->getValue() > 0.5f)
		{
			GennyPatch* patch = static_cast<GennyPatch*>(getVst()->getCurrentPatch());
			_owner->copyPatch(patch);
		}
	}
	else if(control->getTag() == kPasteButton)
	{
		if(control->getValue() > 0.5f)
			_owner->pastePatch();
	}
}

void UIPresetElement::valueChanged()
{
	if(value == 0.0f)
	{
		select();
		return;
	}

	_owner->valueChanged(this);
}

void UIPresetElement::select()
{	
	if(value == 0.0f)
	{
		setValue(1.0f);
		invalid();
	}
}

void UIPresetElement::unselect()
{
	if(value == 1.0f)
	{
		setValue(0.0f);
		invalid();
	}
}

void UIPresetElement::setPatchLink(GennyPatch* patch)
{
	_patch = patch;
	if(_patch == nullptr)
	{
		setVisible(false);
		invalid();
	}
	else
	{
		_label->setText(patch->Name.c_str());
		setVisible(true);
		invalid();
	}
}

void UIPresetElement::setVisible(bool visible)
{
	invalid();
	_label->setVisible(visible);
	_pasteButton->setVisible(visible);
	_copyButton->setVisible(visible);
	_typeDisplay->setValue(visible);
	CCheckBox::setVisible(visible);
	CCheckBox::invalid();
}

void UIPresetElement::draw(CDrawContext* context)
{
	if(isVisible())
		CCheckBox::draw(context);
}

void UIPresetElement::invalid()
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

		_copyButton->setVisible(value == 1.0f && _patch->InstrumentDef.Type != GIType::DAC);
		_copyButton->invalid(); 
		_pasteButton->setVisible(value == 1.0f && _patch->InstrumentDef.Type != GIType::DAC);
		_pasteButton->invalid();
	}

	CCheckBox::invalid();
}
