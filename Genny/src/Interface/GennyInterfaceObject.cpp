#include "GennyInterfaceObject.h"
#include "GennyInterface.h"
#include "GennyVST.h"
#include "Genny2612.h"

GennyInterfaceObject::GennyInterfaceObject(GennyInterfaceObject* parent)
{
	_parent = parent;

	if(_parent != nullptr)
	{
		GennyInterfaceObject* previousParent = nullptr;
		while(parent != nullptr)
		{
			previousParent = parent;
			parent = parent->_parent;
		}

		GennyInterface* gennyInterface = (GennyInterface*)previousParent;
		_interface = gennyInterface;
		_vst = gennyInterface->getVSTOwner();
	}
}

GennyInterfaceObject::~GennyInterfaceObject(void)
{
}

GennyPatch* GennyInterfaceObject::getPatch(int index)
{
	return (GennyPatch*)_vst->getPatch(index);
}

GennyPatch* GennyInterfaceObject::getCurrentPatch()
{
	return (GennyPatch*)_vst->getCurrentPatch();
}

int GennyInterfaceObject::getPatchIndex(VSTPatch* patch)
{
	return _vst->getPatchIndex(patch);
}

IndexBaron* GennyInterfaceObject::getIndexBaron()
{
	return _vst->getCore()->getIndexBaron();
}

int GennyInterfaceObject::getInstrumentIndex(int index)
{
	if(getPatch(1)->Instruments[index] >= 0)
		return getPatch(1)->Instruments[index];
	return index;
}