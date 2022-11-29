#include "UISpinner.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIInstrumentsPanel.h"

UISpinner::UISpinner(CPoint point, GennyInterfaceObject* owner, GennyInstrumentParam param, int op):
	CControl(CRect(point.x, point.y, point.x + ((param == GennyInstrumentParam::GIP_Ch3OpOctave || param == GennyInstrumentParam::GIP_Ch3OpTranspose) ? 40 : 20), point.y + 30), nullptr),
	GennyInterfaceObject(owner),
	_param(param),
	_owner(owner),
	_position(point),
	_upArrow(nullptr),
	_downArrow(nullptr),
	_op(op)
{

}

UISpinner::~UISpinner(void)
{

}

CMouseEventResult UISpinner::onMouseUp(CPoint& where, const CButtonState& buttons)
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

CMouseEventResult UISpinner::onMouseMoved(CPoint& where, const CButtonState& buttons)
{
	if (buttons.isRightButton())
		return kMouseEventHandled;

	return __super::onMouseMoved(where, buttons);
}

CMouseEventResult UISpinner::onMouseDown(CPoint& where, const CButtonState& buttons)
{
	if (buttons.isRightButton())
		return kMouseEventHandled;
	else
		return __super::onMouseDown(where, buttons);
}

CMouseEventResult UISpinner::onMouseEntered(CPoint& where, const CButtonState& buttons)
{
	getInterface()->hoverControl(this);
	return __super::onMouseEntered(where, buttons);
}

CMouseEventResult UISpinner::onMouseExited(CPoint& where, const CButtonState& buttons)
{
	getInterface()->unhoverControl(this);
	return __super::onMouseExited(where, buttons);
}

void UISpinner::setVisible(bool visible)
{
	__super::setVisible(visible);
	_midiLabel->setVisible(visible);
	_upArrow->setVisible(visible);
	_downArrow->setVisible(visible);
}

bool UISpinner::attached (CView* parent)
{
	bool returnValue = CControl::attached(parent);

	CFrame* frame = parent->getFrame();
	IndexBaron* baron = getIndexBaron();
	int index = baron->getInsParamIndex(_param);
	
	tag = index;
	setMax(32.0f);

	if (_param == GennyInstrumentParam::GIP_Ch3OpOctave || _param == GennyInstrumentParam::GIP_Ch3OpTranspose)
	{
		float xoff = 36.0f;
		float yoff = 0.0f;
		_upArrow = new CKickButton(CRect(_position.x + xoff, _position.y + yoff, _position.x + xoff + 8, _position.y + yoff + 14), this, 9999999, 14, UIBitmap(PNG_LITTLEVERTARROWRIGHT));
		frame->addView(_upArrow);

		xoff = 0;
		_downArrow = new CKickButton(CRect(_position.x + xoff, _position.y + yoff, _position.x + xoff + 8, _position.y + yoff + 14), this, 99999999, 14, UIBitmap(PNG_LITTLEVERTARROWLEFT));
		frame->addView(_downArrow);

		xoff = 14;

		_midiLabel = new CTextLabel(CRect(_position.x + xoff, _position.y - 6, _position.x + xoff + 16, _position.y - 4 + 25), "16");
		_midiLabel->setFont(kNormalFontBig);
		_midiLabel->setHoriAlign(kCenterText);
		_midiLabel->getFont()->setStyle(kBoldFace);
		_midiLabel->setFontColor(CColor(16, 20, 16, 255));
		frame->addView(_midiLabel);
		_midiLabel->setMouseableArea(CRect());
		_midiLabel->setBackColor(CColor(0, 0, 0, 0));
		_midiLabel->setFrameColor(CColor(0, 0, 0, 0));
	}
	else
	{
		float xoff = 20.0f;
		_upArrow = new CKickButton(CRect(_position.x + xoff, _position.y, _position.x + xoff + 16, _position.y + 8), this, 9999999, 8, UIBitmap(PNG_LITTLEUPARROW));
		frame->addView(_upArrow);

		_downArrow = new CKickButton(CRect(_position.x + xoff, _position.y + 10, _position.x + xoff + 16, _position.y + 10 + 8), this, 99999999, 8, UIBitmap(PNG_LITTLEDOWNARROW));
		frame->addView(_downArrow);

		_midiLabel = new CTextLabel(CRect(_position.x - 4, _position.y - 6, _position.x + 25, _position.y - 4 + 25), "16");
		_midiLabel->setFont(kNormalFontBig);
		_midiLabel->setHoriAlign(kLeftText);
		_midiLabel->getFont()->setStyle(kBoldFace);
		_midiLabel->setFontColor(CColor(16, 20, 16, 255));
		frame->addView(_midiLabel);
		_midiLabel->setMouseableArea(CRect());
		_midiLabel->setBackColor(CColor(0, 0, 0, 0));
		_midiLabel->setFrameColor(CColor(0, 0, 0, 0));
	}

	reconnect();

	return returnValue;
}

void UISpinner::setValue(float val)
{
	CControl::setValue(val);
	val -= (GennyInstrumentParam_getRange(_param) / 2);
	char buf[4];
	itoa((int)val, buf, 10);
	_midiLabel->setText(buf);
	_midiLabel->invalid();
}

void UISpinner::valueChanged (CControl* control)
{
	if(control->getTag() == 9999999 && control->getValue() > 0.5f)
	{
		if(getValue() < GennyInstrumentParam_getRange(_param))
			setValue(getValue() + 1.0f);
	}
	else if(control->getTag() == 99999999 && control->getValue() > 0.5f)
	{
		if(getValue() > 0.0f)
			setValue(getValue() - 1.0f);
	}
	getInterface()->valueChanged(this);
}

void UISpinner::draw (CDrawContext* pContext)
{
	CControl::setDirty(false);
}

void UISpinner::reconnect()
{
	IndexBaron* baron = getIndexBaron();
	int index = baron->getPatchParamIndex(GPP_SelectedInstrument);

	int selection = getInstrumentIndex(getPatch(0)->SelectedInstrument);
	GennyPatch* selectedPatch = (GennyPatch*)getVst()->getPatch(selection);

	if (_param == GIP_Octave)
		setExtParam(selectedPatch->getExt(GEParam::InsOctave));
	else if(_param == GIP_Transpose)
		setExtParam(selectedPatch->getExt(GEParam::InsTranspose));
	else if (_param == GIP_Ch3OpOctave)
		setExtParam(getCurrentPatch()->getExt(GEParam::Op3SpecialOctave, _op));
	else if (_param == GIP_Ch3OpTranspose)
		setExtParam(getCurrentPatch()->getExt(GEParam::Op3SpecialTranspose, _op));


	/*index = baron->getInsParamIndex(_param);
	setValue(selectedPatch->getFromBaron(baron->getIndex(index)));

	if(_param == GIP_Octave)
		tag = kOctaveStart + selection;
	if(_param == GIP_Transpose)
		tag = kTransposeStart + selection; */
}

bool UISpinner::onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons)	
{		
	if (distance > 0 && getValue() < GennyInstrumentParam_getRange(_param))
	{
		setValue(getValue() + 1.0f);
		valueChanged(this);
	}
	else if (distance < 0 && getValue() > 0.0f)
	{
		setValue(getValue() - 1.0f);
		valueChanged(this);
	}

	return true;
}