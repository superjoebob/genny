#include "VirtualInstrument.h"
#include "Genny2612.h"
#include <algorithm>

VirtualInstrument::VirtualInstrument(void)
	: _currentPatch(NULL)
{

}


VirtualInstrument::~VirtualInstrument(void)
{
}

VSTBase* VirtualInstrument::initializeBase(void* data)
{
	_base = new VSTBase(this, data);
	initialize();
	_base->initialize();
	return _base;
}

void VirtualInstrument::getParameterName (int index, char* text)
{
	text = nullptr;
}

void VirtualInstrument::getParameterValue (int index, char* text)
{
	text = nullptr;
}

void VirtualInstrument::setPatchIndex(int index)
{
	setCurrentPatch(_patches[index]);
}

int VirtualInstrument::getPatchIndex(VSTPatch* patch)
{
	for(size_t i = 0; i < _patches.size(); i++)
	{
		if(_patches[i] == patch)
			return i;
	}
	return -1;
}

std::vector<VSTPatch*> VirtualInstrument::getPatches(std::string category)
{
	bool isRoot = (category == "");
	std::vector<VSTPatch*> patches;
	std::vector<std::string> handledCategories;
	for(int i = 0; i < _patches.size(); i++)
	{
		if((isRoot && _patches[i]->Prefix == "") || (_patches[i]->Prefix == category))
			patches.push_back(_patches[i]);
		else if(isRoot && _patches[i]->Prefix != "")
		{
			//Add the instrument if its the first one from the category
			if(std::find(handledCategories.begin(), handledCategories.end(), _patches[i]->Prefix) == handledCategories.end())
			{
				_patches.push_back(_patches[i]);
				handledCategories.push_back(_patches[i]->Prefix);
			}
		}
	}
	return patches;
}

void VirtualInstrument::setProgramName(char* name)
{
	VSTPatch* current = getCurrentPatch();
	if(current != NULL)
		current->Name = name;
}

void VirtualInstrument::setEditor(VSTEditor* editor)
{
	_base->setEditor(editor);
}

