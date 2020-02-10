#include "UISpinner.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIInstrumentsPanel.h"

UISpinner::UISpinner(CPoint point, GennyInterfaceObject* owner, GennyInstrumentParam param):
	CControl(CRect(point.x, point.y, point.x + 20, point.y + 30), (CControlListener*)owner),
	GennyInterfaceObject(owner),
	_param(param),
	_owner(owner),
	_position(point)
{

}

UISpinner::~UISpinner(void)
{

}

bool UISpinner::attached (CView* parent)
{
	bool returnValue = CControl::attached(parent);

	CFrame* frame = parent->getFrame();
	IndexBaron* baron = getIndexBaron();
	int index = baron->getInsParamIndex(_param);

	tag = index;
	setMax(32.0f);

	float xoff = 20.0f;
	CKickButton* upArrow = new CKickButton(CRect(_position.x + xoff, _position.y, _position.x + xoff + 16, _position.y + 8), this, 9999999, 10, UIBitmap(PNG_LITTLEUPARROW));
	frame->addView(upArrow);

	CKickButton* downArrow = new CKickButton(CRect(_position.x + xoff, _position.y + 10, _position.x + xoff + 16, _position.y + 10 + 8), this, 99999999, 10, UIBitmap(PNG_LITTLEDOWNARROW));
	frame->addView(downArrow);

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


	index = baron->getInsParamIndex(_param);
	setValue(selectedPatch->getFromBaron(baron->getIndex(index)));

	if(_param == GIP_Octave)
		tag = kOctaveStart + selection;
	if(_param == GIP_Transpose)
		tag = kTransposeStart + selection; 
}

bool UISpinner::onWheel (const CPoint& where, const float& distance, const CButtonState& buttons)	
{		
	if(distance > 0 && getValue() < GennyInstrumentParam_getRange(_param))
			setValue(getValue() + 1.0f);
	else if(distance < 0 && getValue() > 0.0f)
			setValue(getValue() - 1.0f);

	return true;
}