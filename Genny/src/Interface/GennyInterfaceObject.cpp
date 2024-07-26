#include "GennyInterfaceObject.h"
#include "GennyInterface.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "Genny2612.h"
#include "lib/platform/win32/win32frame.h"
#include "../resource.h"

GennyInterfaceObject::GennyInterfaceObject(GennyInterfaceObject* parent)
{
	_parent = parent;
	directParent = nullptr;

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

	ContextMenu = nullptr;
}

GennyInterfaceObject::~GennyInterfaceObject(void)
{
	if (ContextMenu != nullptr)
	{
		// destroy our popup menu and all subitems
		int count = GetMenuItemCount((HMENU)ContextMenu);
		while (count > 0) {
			DeleteMenu((HMENU)ContextMenu, count - 1, MF_BYPOSITION);
			count--;
		}
		DestroyMenu((HMENU)ContextMenu);
	}
}

GennyPatch* GennyInterfaceObject::getPatch(int index)
{
	return (GennyPatch*)_vst->getPatch(index);
}

GennyPatch* GennyInterfaceObject::getCurrentPatch()
{
	return (GennyPatch*)_vst->getCurrentPatch();
}

GennyPatch* GennyInterfaceObject::getCurrentInstrument()
{
	return (GennyPatch*)_vst->getPatch(((GennyPatch*)_vst->getPatch(0))->Instruments[((GennyPatch*)_vst->getPatch(0))->SelectedInstrument]);
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
	//if(getPatch(1)->Instruments[index] >= 0)
	//	return getPatch(1)->Instruments[index];
	return index;
}

GennyInterfaceObject* editingObject;
int editingValue;
char editingValueString[80];
BOOL CALLBACK TypeValueProc(HWND hwndDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:

		itoa(editingValue, editingValueString, 10);
		SetDlgItemTextA(hwndDlg, IDC_EDIT1, editingValueString);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			if (!GetDlgItemTextA(hwndDlg, IDC_EDIT1, editingValueString, 80))
				*editingValueString = 0;

			// Fall through. 
		case IDCANCEL:
			EndDialog(hwndDlg, wParam);
			return TRUE;
		}
	}
	return FALSE;
}


int copiedValue = -1;
void GennyInterfaceObject::onMouseUpContext(int tag)
{
	VSTBase* b = getVst()->getBase();

	Win32Frame* winFrame = (Win32Frame*)_interface->getFrame()->getPlatformFrame();
	CPoint mousePos, globalPos;
	winFrame->getCurrentMousePosition(mousePos);
	winFrame->getGlobalPosition(globalPos);
	mousePos = mousePos + globalPos;


	GennyPatch* patch0 = static_cast<GennyPatch*>(getVst()->getPatch(0));
	int numParams = GennyPatch::getNumParameters();
	int patchNum = patch0->SelectedInstrument;

	int paramTag = (numParams * patchNum) + tag;
	if (GennyExtParam::isExtParam(tag))
	{
		//int originalParamsEnd = numParams * getVst()->getNumPatches();
		//int extParamIndex = kExtParamsStart - tag;
		paramTag = tag;
	}



	int learnedInstrumentParamIndex = paramTag;
	IBIndex* idx = getVst()->getCore()->getIndexBaron()->getIndex(tag);
	if (idx != nullptr && idx->global)
		paramTag = tag;

	if (ContextMenu == nullptr)
		ContextMenu = CreatePopupMenu();

	int count = GetMenuItemCount((HMENU)ContextMenu);
	while (count > 0) {
		DeleteMenu((HMENU)ContextMenu, count - 1, MF_BYPOSITION);
		count--;
	}

	unsigned int flags = MF_STRING;

	char nameBuffer[64];
	memset(nameBuffer, 0, 64);
	getVst()->getParameterName(tag, nameBuffer);
	if (strlen(nameBuffer) > 0)
	{
		AppendMenuA((HMENU)ContextMenu, flags | MF_DISABLED, 1, (std::string("-") + std::string(nameBuffer) + std::string("-")).c_str());
		AppendMenuA((HMENU)ContextMenu, MF_SEPARATOR, 0, NULL);
	}

#if BUILD_VST
	AppendMenuA((HMENU)ContextMenu, flags, 1, "MIDI Learn...");
	AppendMenuA((HMENU)ContextMenu, flags, 2, "MIDI Forget");
	AppendMenuA((HMENU)ContextMenu, MF_SEPARATOR, 0, NULL);
	AppendMenuA((HMENU)ContextMenu, flags, 3, "Type Value...");
	AppendMenuA((HMENU)ContextMenu, flags, 4, "Copy Value");

	if (copiedValue >= 0)
		AppendMenuA((HMENU)ContextMenu, flags, 5, "Paste Value");
#else
	b->AdjustParamPopup((HMENU)ContextMenu, learnedInstrumentParamIndex, 0, DefaultMenuID);
	AppendMenuA((HMENU)ContextMenu, MF_SEPARATOR, 0, NULL);
	AppendMenuA((HMENU)ContextMenu, flags, 3, "Type in value (Integer)...");

	if(getVst()->noteControlIsAssigned(0, paramTag))
		AppendMenuA((HMENU)ContextMenu, flags, 6, "Unlink from Note control 1");
	else
		AppendMenuA((HMENU)ContextMenu, flags, 6, "Link to Note control 1");

	if (getVst()->noteControlIsAssigned(1, paramTag))
		AppendMenuA((HMENU)ContextMenu, flags, 7, "Unlink from Note control 2");
	else
		AppendMenuA((HMENU)ContextMenu, flags, 7, "Link to Note control 2");
#endif

	int maxVal = 0;
	GennyExtParam* extParam = nullptr;

	if (GennyExtParam::isExtParam(tag))
	{
		extParam = getVst()->getExtParam(tag);
		if (extParam != nullptr)
		{
			maxVal = extParam->rangeMax;
			paramTag = tag;
		}
	}
	else
	{
		if (idx == nullptr)
			return;

		if (idx->getType() == IBType::IB_YMParam)
			maxVal = YM2612Param_getRange(((IBYMParam*)idx)->getParameter());
	}

	BOOL r = TrackPopupMenu((HMENU)ContextMenu, TPM_RETURNCMD | TPM_RIGHTBUTTON, mousePos.x, mousePos.y, 0, winFrame->getPlatformWindow(), NULL);
	if (r < 500)
	{
		if (r == 1)
		{
			getInterface()->midiLearn(tag);
			b->MidiLearn(tag, paramTag);
		}
		else if (r == 2)
		{
			getInterface()->midiForget(tag);
			b->MidiForget(tag, paramTag);
		}
		else if (r == 3)
		{
			LPWSTR str = MAKEINTRESOURCEW(IDD_DIALOG2);

			MEMORY_BASIC_INFORMATION mbi;
			static int dummyVariable;
			VirtualQuery(&dummyVariable, &mbi, sizeof(mbi));
			HMODULE hMod = (HMODULE)mbi.AllocationBase;

			Win32Frame* winFrame = (Win32Frame*)_interface->getFrame()->getPlatformFrame();



			editingObject = this;

			if (extParam != nullptr)
				editingValue = (int)extParam->get();
			else
				editingValue = (int)getVst()->getCurrentPatch()->getFromBaron(idx);

			if (DialogBoxW(hMod, str, winFrame->getPlatformWindow(), (DLGPROC)TypeValueProc) == IDOK)
			{
				try
				{
					int val = atoi(editingValueString);
					if (val < 0)
						val = 0;
					if (val > maxVal && maxVal != 0)
						val = maxVal;

					if (extParam != nullptr)
					{
						getInterface()->valueChangedExt(extParam, (float)val);
						getInterface()->reconnect();
					}
					else
					{
						getVst()->getCurrentPatch()->setFromBaron(idx, val);
						getInterface()->reconnect();
						for (int i = 0; i < 6; i++)
						{
							if (getVst()->getCore()->getChannelPatch(i) == getVst()->getCurrentPatch())
								getVst()->getCore()->setFromBaron(idx, i, val);
						}
					}
				}
				catch (const std::exception& e)
				{

				}
			}
		}
		else if (r == 4)
		{
			if (extParam != nullptr)
				copiedValue = extParam->get();
			else
				copiedValue = (int)getVst()->getCurrentPatch()->getFromBaron(idx);
		}
		else if (r == 5)
		{
			int val = copiedValue;
			if (val < 0)
				val = 0;
			if (val > maxVal && maxVal != 0)
				val = maxVal;

			if (extParam != nullptr)
			{
				getInterface()->valueChangedExt(extParam, (float)val);
				getInterface()->reconnect();
			}
			else
			{
				getVst()->getCurrentPatch()->setFromBaron(idx, val);
				getInterface()->reconnect();

				for (int i = 0; i < 6; i++)
				{
					if (getVst()->getCore()->getChannelPatch(i) == getVst()->getCurrentPatch())
						getVst()->getCore()->setFromBaron(idx, i, val);
				}
			}
		}
#if !BUILD_VST
		else if (r == 6) //link to note control 1
		{
			if(getVst()->noteControlIsAssigned(0, paramTag))
				getVst()->unassignNoteControl(0, paramTag);
			else
				getVst()->assignNoteControl(0, paramTag);
		}
		else if (r == 7) //link to note control 2
		{
		if (getVst()->noteControlIsAssigned(1, paramTag))
			getVst()->unassignNoteControl(1, paramTag);
		else
			getVst()->assignNoteControl(1, paramTag);
		}
#endif
	}
#if !BUILD_VST
	else if(r != DefaultMenuID)
		b->PlugHost->Dispatcher(b->HostTag, FHD_ParamMenu, paramTag, r - DefaultMenuID);
#endif

}

void GennyInterfaceObject::onMouseDownContext(int tag)
{

}

