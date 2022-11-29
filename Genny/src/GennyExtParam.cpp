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

bool GennyExtParam::isExtParam(int tag)
{
	return tag >= kExtParamsStart && tag <= kExtParamsEnd;
}

int GennyExtParam::getTag() { return (kExtParamsStart + (ins->patchIndex * (int)GEParam::TOTAL_EXT_PARAMS)) + (int)param; }