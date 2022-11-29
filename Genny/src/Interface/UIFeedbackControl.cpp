#include "UIFeedbackControl.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"

UIFeedbackControl::UIFeedbackControl(const CRect& size, UIInstrument* owner):
	CControl(size, owner),
	GennyInterfaceObject(owner),
	_owner(owner)
{
	CFrame* frame = getInterface()->getFrame();
	IndexBaron* baron = getIndexBaron();

	float yoff = 212;
	UIBitmap knobImage(PNG_KNOB);
	CRect knobSize = CRect(0, 0, 26, 26);
	knobSize.offset(52, yoff);

	int index = baron->getYMParamIndex(YM_FB);
	_feedback = new UIKnob(knobSize, this, index, 31, 26.0, knobImage, getInterface());
	//_feedback->setMidiLearnAppearance(UIBitmap(PNG_KNOBRED));
	tag = index;

	frame->addView(_feedback);
	_feedback->setMin(0);
	_feedback->setMax((float)YM2612Param_getRange(YM_FB));
	_feedback->setWheelInc(1.0f / (float)YM2612Param_getRange(YM_FB));

	CRect displaySize = CRect(0, 0, 39, 18);
	displaySize.offset(88.0f, yoff + 4);
	_fbDisplay = new UIImage(displaySize, PNG_FBVAL, true);
	frame->addView(_fbDisplay);

	VSTPatch* patch = getVst()->getCurrentPatch();

	setValue(patch->getFromBaron(baron->getIndex(index)));
}

void UIFeedbackControl::midiLearn()
{
	//_feedback->midiLearn();
}
void UIFeedbackControl::midiForget()
{
	//_feedback->midiForget();
}

UIFeedbackControl::~UIFeedbackControl(void)
{

}

void UIFeedbackControl::setValue(float val)
{
	CControl::setValue(val);
	_feedback->setValue(val);

	int frame = (int)(_feedback->getValue() + 0.5f);
	if(frame >= _fbDisplay->getNumFrames())
		frame = _fbDisplay->getNumFrames() - 1;
	_fbDisplay->setFrame(frame);
}

void UIFeedbackControl::valueChanged (CControl* control)
{
	setValue(control->getValue());
	_owner->valueChanged(control);
}

void UIFeedbackControl::draw (CDrawContext* pContext)
{
	CControl::setDirty(false);
}