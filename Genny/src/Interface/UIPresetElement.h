#pragma once
//#include "vstgui.h"
#include "UIImage.h"
#include "GennyInterfaceObject.h"
class UIPresetsPanel;
struct GennyPatch;
class UIPresetElement : public CCheckBox, public IControlListener, public GennyInterfaceObject
{
public:
	UIPresetElement(const CPoint& vPosition, UIPresetsPanel* vOwner, GennyPatch* vPatch, int vIndex);
	~UIPresetElement(void);
	virtual bool attached (CView* parent);	

	virtual bool onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons);		
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);

	virtual void valueChanged (CControl* control);
	virtual void valueChanged ();

	void unselect();
	void select();

	void setPatchLink(GennyPatch* patch);
	GennyPatch* getPatchLink() { return _patch; }

	virtual void setVisible(bool visible);
	virtual void invalid();
	virtual void draw(CDrawContext* context);

	CLASS_METHODS(UIPresetElement, CCheckBox)

private:
	UIPresetsPanel* _owner;
	CTextLabel* _label;
	bool _selected;
	GennyPatch* _patch;
	CKickButton* _copyButton;
	CKickButton* _pasteButton;
	std::string _category;

	UIImage* _typeDisplay;
};

