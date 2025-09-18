#include "GennyExtParam.h"
#include "Genny2612.h"


const char** GennyExtParam::kDefaultPingPongSettings = new const char* [kNumPingPongSettings]
{
	"Off",
		"LCR",
		"LCRC",
		"LLCCRR",
		"LLCCRRCC",
		"CCLR",
		"LR",
		"LLRR",
		"Custom..."
};

int GennyExtParam::kOriginalParamsEnd = 22770;
int GennyExtParam::kExtParamsEnd = 32727;
int GennyExtParam::kNumPingPongSettings = 9;

bool GennyExtParam::isExtParam(int tag)
{
	return tag >= kOriginalParamsEnd && tag <= kExtParamsEnd;
}

int GennyExtParam::getTag() 
{
	int originalParamsEnd = kOriginalParamsEnd; //(GennyPatch::getNumParameters() * getVst()->getNumPatches());
	int originalExtPatchIndex = (ins->patchIndex * (int)GEParam::TOTAL_EXT_PARAMS) + (int)param;
	return originalParamsEnd + originalExtPatchIndex;
}