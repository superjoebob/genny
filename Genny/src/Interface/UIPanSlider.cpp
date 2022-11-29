#include "UIPanSlider.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIPESelectedInstrument.h"
#include "UIDigitDisplay.h"
#include "UIInstrumentsPanel.h"

UIPanSlider::UIPanSlider(const CPoint& vPosition, int vWidth, GennyInterfaceObject* vOwner, IControlListener* pListener, UIPanSliderType eType, int op):
	CControl(CRect(), pListener),
	GennyInterfaceObject(vOwner),
    _delayLabel(NULL),
	_position(vPosition),
	_type(eType),
	_width(vWidth),
	_op(op)
{

} 

UIPanSlider::~UIPanSlider(void)
{

}

bool UIPanSlider::attached (CView* parent)
{
	bool returnValue = CControl::attached(parent);

	CFrame* frame = getFrame();
	IndexBaron* baron = getIndexBaron();

	UIBitmap sliderKnob((_type == UIPanSliderType::DetuneVert || _type == UIPanSliderType::SamplePitch) ? PNG_LCDSLIDERSMALL : ((_type == UIPanSliderType::Pan) ? PNG_LCDSLIDER2 : PNG_LCDSLIDER));

	CRect sliderSize = CRect(0, 0, _width , 12);
	sliderSize.offset(_position.x, _position.y);

	//if (_glide)
	//	index = 99997;


	if (_type == UIPanSliderType::Pan)
	{
		sliderSize = CRect(0, 0, _width, 16);
		sliderSize.offset(_position.x, _position.y);
	}

	if (_type == UIPanSliderType::DetuneVert)
	{
		sliderSize = CRect(0, 0, 12, _width);
		sliderSize.offset(_position.x, _position.y);
		_slider = new UISlider(sliderSize, this, 0, _position.y, _position.y + (_width - 16), sliderKnob, NULL, getInterface(), CPoint(0, 0), CSliderBase::Style::kVertical | CSliderBase::Style::kBottom);
		frame->addView(_slider);
		_slider->setBackground(NULL);
	}
	else
	{
		_slider = new UISlider(sliderSize, this, 0, _position.x, _position.x + (_width - 16), sliderKnob, NULL, getInterface());
		frame->addView(_slider);
		_slider->setBackground(NULL);
	}

	/*if (_glide)
		_slider->setMax(32);
	else
		_slider->setMax(GennyInstrumentParam_getRange(_delay ? GIP_Delay : GIP_Panning));*/

	//setMax(_slider->getMax());

	if(_type == UIPanSliderType::Delay || _type == UIPanSliderType::Glide || _type == UIPanSliderType::DetuneHor || _type == UIPanSliderType::SamplePitch)
	{
		_slider->setWheelInc(1 / 32.0f);

		int labelX = 646;
		int labelY = _position.y;


		if (_type == UIPanSliderType::Delay || _type == UIPanSliderType::Glide)
		{
			//labelX -= 10;
			labelY -= 13;
			_slider->centerStick = 16;
		}	
		
		if (_type == UIPanSliderType::SamplePitch)
		{
			labelX = 164;
			labelY = 407;
		}

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

	if (_type == UIPanSliderType::DetuneVert || _type == UIPanSliderType::DetuneHor)
	{
		_slider->centerStick = 50;
	}

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
	if (_type == UIPanSliderType::SamplePitch)
		listener->valueChanged(this);
	else
		getInterface()->valueChanged(control);

	if (changing == false && _type != UIPanSliderType::SamplePitch)
	{
		changing = true;
		_slider->setValue((int)(_slider->getValue() + 0.5f));
		changing = false;
	}

	updateLabel();
}

void UIPanSlider::updateLabel()
{
	if (_delayLabel != NULL)
	{
		if (_type == UIPanSliderType::DetuneHor)
		{
			char highBuf[4];
			itoa((int)(int)((_slider->getValue()) - 50), highBuf, 10);
			_delayLabel->setText(highBuf);
			_delayLabel->invalid();
		}
		else
		{
			char highBuf[8];

			if (_type == UIPanSliderType::SamplePitch)
				itoa((int)((_slider->getValue() * 240) - 120), highBuf, 10);
			else
				itoa((int)(_slider->getValue() + 0.5f), highBuf, 10);

			_delayLabel->setText(highBuf);
			_delayLabel->invalid();
		}
	}
}

void UIPanSlider::draw (CDrawContext* pContext)
{
	CControl::setDirty(false);
}

void UIPanSlider::setVisible(bool visible)
{
	if(_delayLabel != nullptr)
		_delayLabel->setVisible(visible);
	_slider->setVisible(visible);
	__super::setVisible(visible);
}

void UIPanSlider::reconnect()
{
	IndexBaron* baron = getIndexBaron();

	int index = baron->getPatchParamIndex(GPP_SelectedInstrument);

	int selection = getInstrumentIndex(getPatch(0)->SelectedInstrument);
	GennyPatch* selectedPatch = (GennyPatch*)getVst()->getPatch(selection);


	if (_type == UIPanSliderType::Delay)
		_slider->setExtParam(selectedPatch->getExt(GEParam::InsDelay));
	else if (_type == UIPanSliderType::Glide)
		_slider->setExtParam(selectedPatch->getExt(GEParam::InsGlide));
	else if (_type == UIPanSliderType::Pan)
		_slider->setExtParam(selectedPatch->getExt(GEParam::InsPan));
	else if (_type == UIPanSliderType::DetuneHor)
		_slider->setExtParam(selectedPatch->getExt(GEParam::InsDetune));
	else if (_type == UIPanSliderType::DetuneVert)
		_slider->setExtParam(getCurrentPatch()->getExt(GEParam::Op3SpecialDetune, _op));

	/*if (_glide)
	{
		setValue(selectedPatch->InstrumentDef.glide);
		tag = 99997;
	}
	else
	{
		index = baron->getInsParamIndex(_delay ? GIP_Delay : GIP_Panning);
		setValue(selectedPatch->getFromBaron(baron->getIndex(index)));
		tag = (_delay ? kDelayStart : kPanningStart) + selection;
		_slider->setTag(index);
	}*/
	updateLabel();
}