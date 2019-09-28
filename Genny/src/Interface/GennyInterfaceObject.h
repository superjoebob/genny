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
	int getPatchIndex(VSTPatch* patch);
	IndexBaron* getIndexBaron();
	virtual void childValueChanged() {}

	int getInstrumentIndex(int index);

	GennyInterface* getInterface() { return _interface; }
	GennyInterfaceObject* directParent;

protected:
	GennyInterfaceObject* _parent;
	GennyInterface* _interface;
	GennyVST* _vst;
	IndexBaron* _indexBaron;
};

