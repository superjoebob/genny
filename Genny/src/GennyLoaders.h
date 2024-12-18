#pragma once
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyData.h"
#include <stdio.h>
#include <windows.h>
class GennyLoaders
{
public:
	static GennyData loadResource(int name, const std::wstring& format);
	static void loadTYI(GennyPatch* patch, GennyData& data);
	static void loadTFI(GennyPatch* patch, GennyData& data);
	static void loadVGI(GennyPatch* patch, GennyData& loadData);
	static GennyData saveTYI(GennyPatch* patch);
	static GennyData saveTFI(GennyPatch* patch);
	static GennyData saveVGI(GennyPatch* patch);
	
	static void loadGEN(GennyPatch* patch, GennyData* loadData);
	static GennyData saveGEN(GennyPatch* patch);

	static unsigned char ssgGennyToReg(unsigned char genny);
	static unsigned char ssgRegToGenny(unsigned char reg);

	static void loadDPACK(GennyPatch* patch, GennyData& data, int* streamPos);

	static void loadGDAC(GennyPatch* patch, GennyData* data);
	static GennyData* saveGDAC(GennyPatch* patch, GennyData* data);
};