#include "UIChannelStatus.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"

UIChannelStatus::UIChannelStatus(UIInstrument* owner):
	CControl(CRect(), owner),
	GennyInterfaceObject(owner),
	_owner(owner)
{
	CFrame* frame = owner->getFrame();
	IndexBaron* baron = getIndexBaron();

	UIBitmap buttonImage(PNG_CHANNELRADIO);
	for(int i = 0; i < 10; i++)
	{
		_active.push_back(false); 

		float extraOff = 0;
		if(i > 5)
			extraOff = 6;

		CRect buttonSize = CRect(0, 0, 12, 20);
		buttonSize.offset(252 + (i * 14 ) + extraOff, 26);  
		_channels.push_back(new CCheckBox(buttonSize, this, kGlobalChannelStart + i, "", buttonImage));
		frame->addView(_channels[i]);


		CRect displaySize = CRect(0, 0, 12, 14);
		displaySize.offset(252.0f + (14 * i) + extraOff, 12.0f);

		UIImage* light = new UIImage(displaySize, i > 5 ? PNG_CHANNELLIGHTORANGE : PNG_CHANNELLIGHT);
		_lights.push_back(light);
		_lightVals.push_back(0.0f);

		light->setFrame(4);
		frame->addView(light);
	}

	reconnect();
	//setValue(owner->getOwner()->getOwner()->getCurrentPatch()->getFromBaron(baron->getIndex(index)));
}

UIChannelStatus::~UIChannelStatus(void)
{

}

void UIChannelStatus::update()
{
	for(int i = 0; i < 10; i++)
	{
		if(_active[i])
			_lightVals[i] = 1.0f;
		else
		{
			if(_lightVals[i] > 0.0f)
				_lightVals[i] -= 0.3f;
			else
				_lightVals[i] = 0.0f;
		}

		_lights[i]->setFrame((1.0f - _lightVals[i]) * 4);
	}
}

void UIChannelStatus::setValue(float val)
{
	/*CControl::setValue(val);

	int index = val * 8;
	for(int i = 0; i < 8; i++)
	{
		if(i != index)
			_algs[i]->setValue(0.0f);
		else
			_algs[i]->setValue(1.0f);
	}
	_algDisplay->setFrame(index);*/
}

void UIChannelStatus::valueChanged (CControl* control)
{
	/*if(control->getValue() == 0.0f)
	{
		control->setValue(1.0f);
		return;
	}
	setValue((float)control->getTag() / 8.0f);*/
	_owner->valueChanged(control);
}

void UIChannelStatus::makeDirty()
{
	for(int i = 0; i < _lights.size(); i++)
	{
		_active[i] = getInterface()->getChannelState(i);
	}
}


void UIChannelStatus::draw (CDrawContext* pContext)
{
	CControl::setDirty(false);
}

void UIChannelStatus::reconnect()
{
	IndexBaron* baron = getIndexBaron();

	_active.clear();
	for(int i = 0; i < 10; i++)
	{
		_active.push_back(false);
		int index = baron->getPatchParamIndex((GennyPatchParam)(GPP_Channel0 + i));
		_channels[i]->setValue(getVst()->getPatch(0)->getFromBaron(baron->getIndex(index)));
	}

}