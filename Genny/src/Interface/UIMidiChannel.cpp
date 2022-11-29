#include "UIMidiChannel.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIInstrumentsPanel.h"

UIMidiChannel::UIMidiChannel(const CPoint& locasticon, GennyInterfaceObject* owner, IControlListener* listener, int op):
	CControl(CRect(locasticon.x, locasticon.y, locasticon.x + 26, locasticon.y + 18), listener),
	GennyInterfaceObject(owner),
	_op(op),
	_upArrow(nullptr),
	_downArrow(nullptr)
{

}

UIMidiChannel::~UIMidiChannel(void)
{

}

CMouseEventResult UIMidiChannel::onMouseUp(CPoint& where, const CButtonState& buttons)
{
	if (buttons.isRightButton())
	{
		onMouseUpContext(tag);
		return kMouseEventHandled;
	}
	else
	{
		return __super::onMouseUp(where, buttons);
	}
}

CMouseEventResult UIMidiChannel::onMouseMoved(CPoint& where, const CButtonState& buttons)
{
	if (buttons.isRightButton())
		return kMouseEventHandled;

	return __super::onMouseMoved(where, buttons);
}

CMouseEventResult UIMidiChannel::onMouseDown(CPoint& where, const CButtonState& buttons)
{
	if (buttons.isRightButton())
		return kMouseEventHandled;
	else
		return __super::onMouseDown(where, buttons);
}

CMouseEventResult UIMidiChannel::onMouseEntered(CPoint& where, const CButtonState& buttons)
{
	getInterface()->hoverControl(this);
	return __super::onMouseEntered(where, buttons);
}

CMouseEventResult UIMidiChannel::onMouseExited(CPoint& where, const CButtonState& buttons)
{
	getInterface()->unhoverControl(this);
	return __super::onMouseExited(where, buttons);
}

bool UIMidiChannel::attached (CView* parent)
{
	bool returnValue = CControl::attached(parent);

	CFrame* frame = getFrame();
	IndexBaron* baron = getIndexBaron();
	int index = baron->getInsParamIndex(GIP_MidiChannel);

	tag = index;
	setMax(16.0f);

	CPoint pos = getViewSize().getTopLeft();
	_upArrow = new CKickButton(CRect(pos.x + 28, pos.y, (pos.x + 28) + 16, pos.y + 8 + 2), this, 9999999, 8, UIBitmap(PNG_LITTLEUPARROW));
	frame->addView(_upArrow);

	_downArrow = new CKickButton(CRect(pos.x + 28, pos.y + 10, (pos.x + 28) + 16, pos.y + 10 + 8), this, 99999999, 8, UIBitmap(PNG_LITTLEDOWNARROW));
	frame->addView(_downArrow);

	_midiLabel = new CTextLabel(CRect(pos.x + 4, pos.y, pos.x + 26, pos.y + 18), "16");
	_midiLabel->setFont(kNormalFontBig);
	_midiLabel->setHoriAlign(kLeftText);
	_midiLabel->getFont()->setStyle(kBoldFace);
	_midiLabel->setFontColor(CColor(16, 20, 16, 255));
	frame->addView(_midiLabel);
	_midiLabel->setMouseableArea(CRect());
	_midiLabel->setBackColor(CColor(0, 0, 0, 0));
	_midiLabel->setFrameColor(CColor(0, 0, 0, 0));

	reconnect();

	return returnValue;
}

void UIMidiChannel::setValue(float val)
{
	CControl::setValue(val);
	if (_op != -1)
	{
		if (val == 0)
		{
			_midiLabel->setText("-");
			_midiLabel->invalid();
		}
		else
		{
			char buf[4];
			itoa((int)val, buf, 10);
			_midiLabel->setText(buf);
			_midiLabel->invalid();
		}
	}
	else 
	{
		char buf[4];
		itoa((int)val + 1, buf, 10);
		_midiLabel->setText(buf);
		_midiLabel->invalid();
	}
}

void UIMidiChannel::valueChanged (CControl* control)
{
	if (_extParam == nullptr)
		return;
	
	if(control->getTag() == 9999999 && control->getValue() > 0.5f)
	{
		if(getValue() < _extParam->rangeMax)
			setValue(getValue() + 1.0f);

		getInterface()->valueChanged(this);
	}
	else if(control->getTag() == 99999999 && control->getValue() > 0.5f)
	{
		if(getValue() > 0.0f)
			setValue(getValue() - 1.0f);

		getInterface()->valueChanged(this);
	}
}

void UIMidiChannel::draw (CDrawContext* pContext)
{
	CControl::setDirty(false);
}

void UIMidiChannel::reconnect()
{
	int selection = getInstrumentIndex(getPatch(0)->SelectedInstrument);
	GennyPatch* selectedPatch = (GennyPatch*)getVst()->getPatch(selection);

	if(_op == -1)
		setExtParam(selectedPatch->getExt(GEParam::InsMidiChannel));
	else
	{
		setExtParam(getCurrentPatch()->getExt(GEParam::Op3SpecialMidi, _op));
		setVisible(getCurrentPatch()->InstrumentDef.Ch3Special);
	}
}

void UIMidiChannel::setVisible(bool visible)
{
	__super::setVisible(visible);
	_midiLabel->setVisible(visible);
	_upArrow->setVisible(visible);
	_downArrow->setVisible(visible);
}

bool UIMidiChannel::onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons)	
{		
	if (_extParam == nullptr)
		return true;

	if (distance > 0 && getValue() < _extParam->rangeMax)
	{
		setValue(getValue() + 1.0f);
		getInterface()->valueChanged(this);
	}
	else if (distance < 0 && getValue() > 0.0f)
	{
		setValue(getValue() - 1.0f);
		getInterface()->valueChanged(this);
	}

	return true;
}