#pragma once
#include "UIImage.h"
#include "UICheckBoxNum.h"
#include "UIPresetsAndInstrumentsPanel.h"
#include "GennyLoaders.h"
class UIPresetElement;
struct GennyPatch;
class UIPresetsPanel : public CControl, public IControlListener, public GennyInterfaceObject
{
public:
	UIPresetsPanel(const CRect& size, UIPresetsAndInstrumentsPanel* owner);
	~UIPresetsPanel(void);

	virtual void valueChanged (CControl* control);
	virtual void draw (CDrawContext* pContext);
	virtual void setDirty(bool dirty = true);

	UIPresetsAndInstrumentsPanel* getOwner() { return _owner; }
	CFrame* getFrame() { return _owner->getFrame(); }
	virtual bool onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons);

	void copyPatch(GennyPatch* patch);
	void pastePatch();
	void startLogging();

	void reconnect();
	void setSelection(int index);
	void reorganize();

	void setVisible(bool visible);

	void addConfirmDialog();

	CLASS_METHODS(UIPresetsPanel, CView)

private:
	UIPresetsAndInstrumentsPanel* _owner;
	CSlider* _scrollBar;
	std::vector<UIPresetElement*> _elements;
	int _selection;
	int _topItem;
	bool _didScroll;
	bool _initialized;

	CKickButton* _upArrow;
	CKickButton* _downArrow;
	GennyData _copyData;
	std::string _copyName;
	std::string _category;

	std::vector<VSTPatch*> _patches;

	std::vector<CView*> _confirmDialog;
	bool _loggingMode;
};

