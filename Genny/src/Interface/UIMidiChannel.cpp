#include "UIMidiChannel.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIInstrumentsPanel.h"

UIMidiChannel::UIMidiChannel(UIInstrumentsPanel* owner):
	CControl(CRect(662, 162, 662 + 26, 162 + 18), owner),
	GennyInterfaceObject(owner),
	_owner(owner)
{

}

UIMidiChannel::~UIMidiChannel(void)
{

}

bool UIMidiChannel::attached (CView* parent)
{
	bool returnValue = CControl::attached(parent);

	CFrame* frame = _owner->getFrame();
	IndexBaron* baron = _owner->getIndexBaron();
	int index = baron->getInsParamIndex(GIP_MidiChannel);

	tag = index;
	setMax(16.0f);

	float xoff = 236;
	float yoff = -4;

	CKickButton* upArrow = new CKickButton(CRect(454 + xoff, 164 + yoff + 2, 454 + xoff + 16, 164 + yoff + 8 + 2), this, 9999999, 10, UIBitmap(PNG_LITTLEUPARROW));
	frame->addView(upArrow);

	CKickButton* downArrow = new CKickButton(CRect(454 + xoff, 164 + yoff + 12, 454 + xoff + 16, 164 + yoff + 12 + 8), this, 99999999, 10, UIBitmap(PNG_LITTLEDOWNARROW));
	frame->addView(downArrow);

	_midiLabel = new CTextLabel(CRect(430 + xoff, (162 + yoff) - 4, 430 + xoff + 25, 162 + yoff + 25), "16");
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

	char buf[3];
	itoa((int)val + 1, buf, 10);
	_midiLabel->setText(buf);
	_midiLabel->invalid();
}

void UIMidiChannel::valueChanged (CControl* control)
{
	if(control->getTag() == 9999999 && control->getValue() > 0.5f)
	{
		if(getValue() < 15.0f)
			setValue(getValue() + 1.0f);
	}
	else if(control->getTag() == 99999999 && control->getValue() > 0.5f)
	{
		if(getValue() > 0.0f)
			setValue(getValue() - 1.0f);
	}
	getInterface()->valueChanged(this);
}

void UIMidiChannel::draw (CDrawContext* pContext)
{
	CControl::setDirty(false);
}

void UIMidiChannel::reconnect()
{
	int selection = getInstrumentIndex(getPatch(0)->SelectedInstrument);
	GennyPatch* selectedPatch = (GennyPatch*)getVst()->getPatch(selection);

	setValue(selectedPatch->InstrumentDef.MidiChannel);

	tag = kMidiChannelStart + selection;
}

bool UIMidiChannel::onWheel (const CPoint& where, const float& distance, const CButtonState& buttons)	
{		
	if(distance > 0 && getValue() < 15.0f)
			setValue(getValue() + 1.0f);
	else if(distance < 0 && getValue() > 0.0f)
			setValue(getValue() - 1.0f);

	return true;
}