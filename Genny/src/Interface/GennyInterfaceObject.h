#pragma once
class GennyVST;
class GennyInterface;
struct VSTPatch;
class IndexBaron;
struct GennyPatch;
class GennyInterfaceObject
{
public:
	GennyInterfaceObject(GennyInterfaceObject* parent);
	~GennyInterfaceObject(void);

	GennyVST* getVst() { return _vst; }
	GennyPatch* getPatch(int index);
	GennyPatch* getCurrentPatch();
	GennyPatch* getCurrentInstrument();
	int getPatchIndex(VSTPatch* patch);
	IndexBaron* getIndexBaron();
	virtual void childValueChanged() {}

	virtual void onMouseUpContext(int tag);
	virtual void onMouseDownContext(int tag);

	int getInstrumentIndex(int index);

	GennyInterface* getInterface() { return _interface; }
	GennyInterfaceObject* directParent;

protected:
	GennyInterfaceObject* _parent;
	GennyInterface* _interface;
	GennyVST* _vst;
	IndexBaron* _indexBaron;
	void* ContextMenu;
};

