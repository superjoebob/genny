#include "UIPanSlider.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIPESelectedInstrument.h"
#include "UIDigitDisplay.h"
#include "UIInstrumentsPanel.h"

UIPanSlider::UIPanSlider(const CPoint& vPosition, int vWidth, UIPESelectedInstrument* vOwner, bool vDelay):
	CControl(CRect(), vOwner),
	GennyInterfaceObject(vOwner),
	_owner(vOwner),
    _delayLabel(NULL),
	_position(vPosition),
	_delay(vDelay),
	_width(vWidth)
{

}

UIPanSlider::~UIPanSlider(void)
{

}

bool UIPanSlider::attached (CView* parent)
{
	bool returnValue = CControl::attached(parent);

	CFrame* frame = _owner->getOwner()->getFrame();
	IndexBaron* baron = getIndexBaron();

	UIBitmap sliderKnob(PNG_LCDSLIDER);
	setMax(100);

	
	int index = baron->getInsParamIndex(_delay ? GIP_Delay : GIP_Panning);
	CRect sliderSize = CRect(0, 0, _width , 12);
	sliderSize.offset(_position.x, _position.y);
	_slider = new UISlider(sliderSize, this, index, _position.x, _position.x + (_width - 16), sliderKnob, NULL, getInterface());
	frame->addView(_slider);
	_slider->setBackground(NULL);

	_slider->setMax(GennyInstrumentParam_getRange(_delay ? GIP_Delay : GIP_Panning));

	if(_delay)
	{
		_slider->setWheelInc(1 / 32.0f);

		int labelX = 646;
		int labelY = 262 - 22 - 18 + 2 + 28;
		_delayLabel = new CTextLabel(CRect(labelX, labelY, labelX + 60, labelY + 12), "127");
		_delayLabel->setFont(kNormalFontBig);
		_delayLabel->setHoriAlign(kLeftText);
		_delayLabel->getFont()->setStyle(kBoldFace);
		_delayLabel->setFontColor(CColor(16, 20, 16, 255));
		frame->addView(_delayLabel);
		_delayLabel->setMouseableArea(CRect());
		_delayLabel->setBackColor(CColor(0, 0, 0, 0));
		_delayLabel->setFrameColor(CColor(0, 0, 0, 0));
		_delayLabel->setHoriAlign(CHoriTxtAlign::kRightText);
		_delayLabel->invalid();
	}
	else
		_slider->centerStick = 127;

	reconnect();

	return returnValue;
}

void UIPanSlider::setValue(float val)
{
	_slider->setValue(val);
} 

float UIPanSlider::getValue() const
{
	float realVal = _slider->getValue();
	return realVal;
	
}

bool changing = false;
void UIPanSlider::valueChanged (CControl* control)
{
	_owner->getOwner()->getOwner()->valueChanged(this);

	if(changing == false)
	{
		changing = true;
		_slider->setValue((int)(_slider->getValue() + 0.5f));
		changing = false;
	}

	if(_delayLabel != NULL)
	{
		char highBuf[4];
		itoa((int)(int)(_slider->getValue() + 0.5f), highBuf, 10);
		_delayLabel->setText(highBuf);
		_delayLabel->invalid();
	}
}

void UIPanSlider::draw (CDrawContext* pContext)
{
	CControl::setDirty(false);
}

void UIPanSlider::reconnect()
{
	IndexBaron* baron = getIndexBaron();

	int index = baron->getPatchParamIndex(GPP_SelectedInstrument);

	int selection = getInstrumentIndex(getPatch(0)->SelectedInstrument);
	GennyPatch* selectedPatch = (GennyPatch*)getVst()->getPatch(selection);


	index = baron->getInsParamIndex(_delay ? GIP_Delay : GIP_Panning);
	setValue(selectedPatch->getFromBaron(baron->getIndex(index)));
	tag = (_delay ? kDelayStart : kPanningStart) + selection;
	_slider->setTag(index);

	if(_delayLabel != NULL)
	{
		char highBuf[4];
		itoa((int)(int)(_slider->getValue() + 0.5f), highBuf, 10);
		_delayLabel->setText(highBuf);
		_delayLabel->invalid();
	}
}