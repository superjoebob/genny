#include "UIAlgorithmSelector.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"

UIAlgorithmSelector::UIAlgorithmSelector(const CRect& size, UIInstrument* owner):
	CControl(size, owner),
	GennyInterfaceObject(owner),
	_owner(owner)
{
	CFrame* frame = getInterface()->getFrame();
	IndexBaron* baron = owner->getIndexBaron();
	int index = baron->getYMParamIndex(YM_ALG);
	tag = index;

	UIBitmap buttonImage(PNG_LITTLEBUTTON);

	for(int i = 0; i < 8; i++)
	{
		CRect buttonSize = CRect(0, 0, 28, 28);
		buttonSize.offset(170 + (i * 28), 138);
		_algs.push_back(new UICheckBoxNum(buttonSize, i, this, i, buttonImage, _interface, false, 16L, 0, this));
		frame->addView(_algs[i]);

		if(i == 0)
			_algs[i]->setValue(1.0f);
	}

	CRect displaySize = CRect(0, 0, 220, 74);
	displaySize.offset(164, 178);
	_algDisplay = new UIImage(displaySize, PNG_ALGVAL, true);
	frame->addView(_algDisplay);

	setMin(0);
	setMax(YM2612Param_getRange(YM_ALG));

	setValue(getCurrentPatch()->getFromBaron(baron->getIndex(index)));
}

UIAlgorithmSelector::~UIAlgorithmSelector(void)
{

}

bool UIAlgorithmSelector::onWheel(const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons)
{
	IndexBaron* baron = _owner->getIndexBaron();
	int index = baron->getYMParamIndex(YM_ALG);
	if (distance > 0)
	{
		setValue(getCurrentPatch()->getFromBaron(baron->getIndex(index)) + 1);
		_owner->valueChanged(this);
	}
	else if (distance < 0)
	{
		setValue(getCurrentPatch()->getFromBaron(baron->getIndex(index)) - 1);
		_owner->valueChanged(this);
	}

	return __super::onWheel(where, axis, distance, buttons);
}


void UIAlgorithmSelector::setValue(float val)
{
	CControl::setValue(val);

	int index = (int)(val + 0.5f);
	if (index < 0)
		index = 0;
	if (index > 7)
		index = 7;

	for(int i = 0; i < 8; i++)
	{
		if(i != index)
			_algs[i]->setValue(0.0f);
		else
			_algs[i]->setValue(1.0f);
	}
	_algDisplay->setFrame(index);
}

void UIAlgorithmSelector::valueChanged (CControl* control)
{
	if(control->getValue() == 0.0f)
	{
		control->setValue(1.0f);
		return;
	}
	setValue((float)control->getTag());
	_owner->valueChanged(this);
}

void UIAlgorithmSelector::draw (CDrawContext* pContext)
{
	CControl::setDirty(false);
}