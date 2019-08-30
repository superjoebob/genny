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
	CFrame* frame = owner->getFrame();
	IndexBaron* baron = owner->getIndexBaron();
	int index = baron->getYMParamIndex(YM_ALG);
	tag = index;

	UIBitmap buttonImage(PNG_LITTLEBUTTON);

	for(int i = 0; i < 8; i++)
	{
		CRect buttonSize = CRect(0, 0, 28, 28);
		buttonSize.offset(168 + (i * 28), 146);
		_algs.push_back(new UICheckBoxNum(buttonSize, i, this, i, buttonImage, _interface));
		frame->addView(_algs[i]);

		if(i == 0)
			_algs[i]->setValue(1.0f);
	}

	CRect displaySize = CRect(0, 0, 220, 74);
	displaySize.offset(164.0f, 152.0f + 44);
	_algDisplay = new UIImage(displaySize, PNG_ALGVAL, true);
	frame->addView(_algDisplay);

	setMin(0);
	setMax(YM2612Param_getRange(YM_ALG));

	setValue(getCurrentPatch()->getFromBaron(baron->getIndex(index)));
}

UIAlgorithmSelector::~UIAlgorithmSelector(void)
{

}

void UIAlgorithmSelector::setValue(float val)
{
	CControl::setValue(val);

	int index = (int)(val + 0.5f);
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