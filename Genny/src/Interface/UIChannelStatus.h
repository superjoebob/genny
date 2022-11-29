#pragma once
#include "UIInstrument.h"
#include "UIImage.h"
#include "UICheckBoxNum.h"
class UIChannelStatus : public CControl, public IControlListener, public GennyInterfaceObject
{
public:
	UIChannelStatus(UIInstrument* owner);
	~UIChannelStatus(void);
	void update();
	void setValue(float val);
	virtual void valueChanged (CControl* control);
	virtual void draw (CDrawContext* pContext);

	void makeDirty();
	void reconnect();

	CLASS_METHODS(UIChannelStatus, CControl)

private:
	UIInstrument* _owner;

	std::vector<CCheckBox*> _channels; 
	std::vector<UIImage*> _lights;
	std::vector<float> _lightVals;
	std::vector<bool> _active;
};

