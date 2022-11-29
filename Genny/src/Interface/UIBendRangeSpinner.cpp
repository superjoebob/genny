#include "UIBendRangeSpinner.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIInstrumentsPanel.h"

UIBendRangeSpinner::UIBendRangeSpinner(CPoint point, GennyInterfaceObject* owner):
	CControl(CRect(point.x, point.y, point.x + 20, point.y + 30), (IControlListener*)owner),
	GennyInterfaceObject(owner),
	_owner(owner),
	_position(point)
{

}

UIBendRangeSpinner::~UIBendRangeSpinner(void)
{

}

bool UIBendRangeSpinner::attached (CView* parent)
{
	bool returnValue = CControl::attached(parent);

	CFrame* frame = parent->getFrame();
	setMax(48);

	float xoff = 20.0f;
	_upButton = new CKickButton(CRect(_position.x + xoff, _position.y, _position.x + xoff + 16, _position.y + 8), this, 9999999, 8, UIBitmap(PNG_LITTLEUPARROW));
	frame->addView(_upButton);

	_downButton = new CKickButton(CRect(_position.x + xoff, _position.y + 10, _position.x + xoff + 16, _position.y + 10 + 8), this, 99999999, 8, UIBitmap(PNG_LITTLEDOWNARROW));
	frame->addView(_downButton);

	_midiLabel = new CTextLabel(CRect(_position.x - 4, _position.y - 8, _position.x + 25, _position.y - 4 + 25), "16");
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

void UIBendRangeSpinner::setValue(float val)
{
	CControl::setValue(val);
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

	_vst->bendRange = (int)getValue();
}

void UIBendRangeSpinner::valueChanged (CControl* control)
{
	if(control->getTag() == 9999999 && control->getValue() > 0.5f)
	{
		if(getValue() < 49)
			setValue(getValue() + 1.0f);
	}
	else if(control->getTag() == 99999999 && control->getValue() > 0.5f)
	{
		if(getValue() > 1)
			setValue(getValue() - 1.0f);
	}

	_vst->bendRange = (int)getValue();
}

void UIBendRangeSpinner::draw (CDrawContext* pContext)
{
	CControl::setDirty(false);
}

void UIBendRangeSpinner::reconnect()
{
	setValue(_vst->bendRange);
}

void UIBendRangeSpinner::setVisible(bool visible)
{
	_midiLabel->setVisible(visible);
	_upButton->setVisible(visible);
	_downButton->setVisible(visible);
}

bool UIBendRangeSpinner::onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons)
{		
	if (axis == CMouseWheelAxis::kMouseWheelAxisY)
	{
		if (distance > 0 && getValue() < 17)
			setValue(getValue() + 1.0f);
		else if (distance < 0 && getValue() > 0)
			setValue(getValue() - 1.0f);

		return true;
	}
	return false;
}