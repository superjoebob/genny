#include "GennyLoaders.h"
#include <windows.h>

unsigned char GennyLoaders::ssgGennyToReg(unsigned char genny)
{
	if(genny == 1)
		return 8;
	if(genny == 2)
		return 9;
	if(genny == 3)
		return 10;
	if(genny == 4)
		return 11;
	if(genny == 5)
		return 12;
	if(genny == 6)
		return 13;
	if(genny == 7)
		return 14;
	if(genny == 8)
		return 15;
	return 0;
}

unsigned char GennyLoaders::ssgRegToGenny(unsigned char reg)
{
	if(reg == 8)
		return 1;
	if(reg == 9)
		return 2;
	if(reg == 10)
		return 3;
	if(reg == 11)
		return 4;
	if(reg == 12)
		return 5;
	if(reg == 13)
		return 6;
	if(reg == 14)
		return 7;
	if(reg == 15)
		return 8;
	return 0;
}

GennyData GennyLoaders::loadResource(int name, const std::wstring& format)
{
	MEMORY_BASIC_INFORMATION mbi; 
	static int dummyVariable; 
	VirtualQuery( &dummyVariable, &mbi, sizeof(mbi) ); 
	HMODULE hMod = (HMODULE)mbi.AllocationBase; 

	LPWSTR s = MAKEINTRESOURCEW(name);
	HRSRC hRes = FindResourceW(hMod, s, format.c_str());
	HGLOBAL hMem = LoadResource(hMod, hRes);
	DWORD size = SizeofResource(hMod, hRes);

	GennyData data;
	data.size = size;
	data.handle = hMem;
	data.data = (char*)LockResource(hMem);

	return data;
}

int dtRevTable[8]={7, 6, 5, 0, 1, 2, 3, 0};
int dtTable[8] = {3, 4, 5, 6, 4, 2, 1, 0};
void GennyLoaders::loadTYI(GennyPatch* patch, GennyData& info)
{
	char* data = info.data;
	unsigned char val = 0;
	int index = 0;
	for( int i = 0; i < 4; i++ )
	{
		val = data[index];
		index++;

		//DT_MUL
		//------------------------------
		//  0   1   2   3   4   5   6   7
		//      |  DT   |   |    MUL    |
		unsigned char mul = val & 15;
		unsigned char dt = (val >> 4) & 7;

		*patch->InstrumentDef.Data.getParameter(YM_MUL, 0, i) = (float)mul;
		*patch->InstrumentDef.Data.getParameter(YM_DT, 0, i) = (float)dt;
	}

	//TL
	for( int i = 0; i < 4; i++ )
	{
		val = data[index];
		index++;
		//TL
		//------------------------------
		//  0   1   2   3   4   5   6   7
		//      |          TL           |
		unsigned char tl = val & 127;
		*patch->InstrumentDef.Data.getParameter(YM_TL, 0, i) = 127 - (float)tl;
	}

	//RS_AR
	for( int i = 0; i < 4; i++ )
	{
		val = data[index];
		index++;
		//RS_AR
		//------------------------------
		//  0   1   2   3   4   5   6   7
		//  |RS |  |X|  |      AR       |
		unsigned char rs = (val >> 6) & 3;
		unsigned char ar = val & 31;
		*patch->InstrumentDef.Data.getParameter(YM_KS, 0, i) = (float)rs;
		*patch->InstrumentDef.Data.getParameter(YM_AR, 0, i) = 31 - (float)ar;
	}

	//AM_DR1
	for( int i = 0; i < 4; i++ )
	{
		val = data[index];
		index++;
		//AM_DR1
		//------------------------------
		//  0   1   2   3   4   5   6   7
		//|AM|          |      DR1      |
		unsigned char am = (val >> 7) & 1;
		unsigned char dr1 = val & 31;
		*patch->InstrumentDef.Data.getParameter(YM_AM, 0, i) = (float)am;
		*patch->InstrumentDef.Data.getParameter(YM_DR, 0, i) = 31 - (float)dr1;
	}

	//DR2
	for( int i = 0; i < 4; i++ )
	{
		val = data[index];
		index++;
		unsigned char dr2 = val;
		*patch->InstrumentDef.Data.getParameter(YM_SR, 0, i) = (float)dr2;
	}

	//DL1_RR
	for( int i = 0; i < 4; i++ )
	{
		val = data[index];
		index++;
		//DL1_RR
		//------------------------------
		//  0   1   2   3   4   5   6   7
		//  |    DL1    |   |     RR    |
		unsigned char dl1 = (val >> 4) & 15;
		unsigned char rr = val & 15;
		*patch->InstrumentDef.Data.getParameter(YM_SL, 0, i) = 15 - (float)dl1;
		*patch->InstrumentDef.Data.getParameter(YM_RR, 0, i) = 15 - (float)rr;
	}

	//SSG-EG
	for( int i = 0; i < 4; i++ )
	{
		val = data[index];
		*patch->InstrumentDef.Data.getParameter(YM_SSG, 0, i) = (float)GennyLoaders::ssgRegToGenny(val);
		index++;
	}

	val = data[index];
	index++;
	//FB_ALGORITHM
	//------------------------------
	//  0   1   2   3   4   5   6   7
	//          |   FB  |   |  ALG  |
	unsigned char fb = (val >> 3) & 7;
	unsigned char alg = val & 7;
	*patch->InstrumentDef.Data.getParameter(YM_FB, 0, -1) = (float)fb;
	*patch->InstrumentDef.Data.getParameter(YM_ALG, 0, -1) = (float)alg;

	val = data[index];
	index++;
	//AMS_FMS
	//------------------------------
	//  0   1   2   3   4   5   6   7
	// |L| |R|  |AMS|       |  FMS  |
	unsigned char ams = (val >> 4) & 3;
	unsigned char fms = val & 7;
	*patch->InstrumentDef.Data.getParameter(YM_AMS, 0, -1) = (float)ams;
	*patch->InstrumentDef.Data.getParameter(YM_FMS, 0, -1) = (float)fms;

	if (ams > 0 || fms > 0)
		patch->InstrumentDef.LFOEnable = true;

	
	patch->InstrumentDef.Type = GIType::FM;
	patch->InstrumentDef.Ch3Special = false;
}

GennyData GennyLoaders::saveTYI(GennyPatch* patch)
{
	GennyData dat;
	for( int i = 0; i < 4; i++ )
	{
		unsigned char mul = patch->InstrumentDef.Data.getParameterChar(YM_MUL, 0, i);
		unsigned char dt = patch->InstrumentDef.Data.getParameterChar(YM_DT, 0, i);
		unsigned char dtmul = (dt << 4) | mul;
		dat.writeByte(dtmul);
	}

	for( int i = 0; i < 4; i++ )
	{
		dat.writeByte(127 - patch->InstrumentDef.Data.getParameterChar(YM_TL, 0, i));
	}

	for( int i = 0; i < 4; i++ )
	{

		unsigned char rs = patch->InstrumentDef.Data.getParameterChar(YM_KS, 0, i);
		unsigned char ar = 31 - patch->InstrumentDef.Data.getParameterChar(YM_AR, 0, i);
		unsigned char rsar = (rs << 6) | ar;
		dat.writeByte(rsar);
	}

	for( int i = 0; i < 4; i++ )
	{
		unsigned char am = patch->InstrumentDef.Data.getParameterChar(YM_AM, 0, i);
		unsigned char dr1 = 31 - patch->InstrumentDef.Data.getParameterChar(YM_DR, 0, i);
		unsigned char amdr = (am << 7) | dr1;
		dat.writeByte(amdr);
	}

	for( int i = 0; i < 4; i++ )
	{
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_SR, 0, i));
	}

	for( int i = 0; i < 4; i++ )
	{
		unsigned char dl1 = 15 - patch->InstrumentDef.Data.getParameterChar(YM_SL, 0, i);
		unsigned char rr = 15 - patch->InstrumentDef.Data.getParameterChar(YM_RR, 0, i);
		unsigned char dlrr = (dl1 << 4) | rr;
		dat.writeByte(dlrr);
	}

	for( int i = 0; i < 4; i++ )
	{
		dat.writeByte(GennyLoaders::ssgGennyToReg(patch->InstrumentDef.Data.getParameterChar(YM_SSG, 0, i)));
	}

	unsigned char fb = patch->InstrumentDef.Data.getParameterChar(YM_FB, 0, -1);
	unsigned char alg = patch->InstrumentDef.Data.getParameterChar(YM_ALG, 0, -1);
	unsigned char fbalg = (fb << 3) | alg;
	dat.writeByte(fbalg);

	unsigned char ams = patch->InstrumentDef.Data.getParameterChar(YM_AMS, 0, -1);
	unsigned char fms = patch->InstrumentDef.Data.getParameterChar(YM_FMS, 0, -1);
	unsigned char amfm = (ams << 4) | fms;
	dat.writeByte(amfm);

	dat.writeByte('Y');
	dat.writeByte('I');
	return dat;
}

void GennyLoaders::loadTFI(GennyPatch* patch, GennyData& info)
{
	if (patch == nullptr)
		throw std::exception("Cannot load information into null patch!");

	char* data = info.data;
	unsigned char val = 0;
	int index = 0;

	//FB_ALGORITHM
	//------------------------------
	unsigned char alg = data[index];
	index++;
	unsigned char fb = data[index];
	index++;
	*patch->InstrumentDef.Data.getParameter(YM_FB, 0, -1) = (float)fb;
	*patch->InstrumentDef.Data.getParameter(YM_ALG, 0, -1) = (float)alg;

	for( int i = 0; i < 4; i++ )
	{
		unsigned char mul = data[index];
		index++;
		*patch->InstrumentDef.Data.getParameter(YM_MUL, 0, i) = (float)mul;

		unsigned char dt = data[index];
		index++;
		*patch->InstrumentDef.Data.getParameter(YM_DT, 0, i) = (float)dtRevTable[dt];

		unsigned char tl = data[index];
		index++;
		*patch->InstrumentDef.Data.getParameter(YM_TL, 0, i) = 127 - (float)tl;

		unsigned char rs = data[index];
		index++;
		*patch->InstrumentDef.Data.getParameter(YM_KS, 0, i) = (float)rs;

		unsigned char ar = data[index];
		index++;
		*patch->InstrumentDef.Data.getParameter(YM_AR, 0, i) = 31 - (float)ar;

		unsigned char dr1 = data[index];
		index++;
		*patch->InstrumentDef.Data.getParameter(YM_DR, 0, i) = 31 - (float)dr1;

		unsigned char dr2 = data[index];
		index++;
		*patch->InstrumentDef.Data.getParameter(YM_SR, 0, i) = (float)dr2;

		unsigned char rr = data[index];
		index++;
		*patch->InstrumentDef.Data.getParameter(YM_RR, 0, i) = 15 - (float)rr;

		unsigned char dl1 = data[index];
		index++;
		*patch->InstrumentDef.Data.getParameter(YM_SL, 0, i) = 15 - (float)dl1;

		unsigned char ssg = data[index];
		index++;
		*patch->InstrumentDef.Data.getParameter(YM_SSG, 0, i) = (float)GennyLoaders::ssgRegToGenny(ssg);
	}

	patch->InstrumentDef.Type = GIType::FM;
	patch->InstrumentDef.Ch3Special = false;
}

GennyData GennyLoaders::saveTFI(GennyPatch* patch)
{
	GennyData dat;

	dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_ALG, 0, -1));
	dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_FB, 0, -1));

	for( int i = 0; i < 4; i++ )
	{
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_MUL, 0, i));
		dat.writeByte(dtTable[patch->InstrumentDef.Data.getParameterChar(YM_DT, 0, i)]);
		dat.writeByte(127 - patch->InstrumentDef.Data.getParameterChar(YM_TL, 0, i));
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_KS, 0, i));
		dat.writeByte(31 - patch->InstrumentDef.Data.getParameterChar(YM_AR, 0, i));
		dat.writeByte(31 - patch->InstrumentDef.Data.getParameterChar(YM_DR, 0, i));
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_SR, 0, i));
		dat.writeByte(15 - patch->InstrumentDef.Data.getParameterChar(YM_RR, 0, i));
		dat.writeByte(15 - patch->InstrumentDef.Data.getParameterChar(YM_SL, 0, i));
		dat.writeByte(GennyLoaders::ssgGennyToReg(patch->InstrumentDef.Data.getParameterChar(YM_SSG, 0, i)));
	}

	return dat;
}

void GennyLoaders::loadVGI(GennyPatch* patch, GennyData& info)
{
	if (patch == nullptr)
		throw std::exception("Cannot load information into null patch!");

	char* data = info.data;
	unsigned char val = 0;
	int index = 0;

	unsigned char alg = data[index];
	index++;
	unsigned char fb = data[index];
	index++;
	*patch->InstrumentDef.Data.getParameter(YM_FB, 0, -1) = (float)fb;
	*patch->InstrumentDef.Data.getParameter(YM_ALG, 0, -1) = (float)alg;


	unsigned char amfm = data[index];
	index++;

	unsigned char ams = (amfm >> 4) & 3;
	unsigned char fms = amfm & 7;
	*patch->InstrumentDef.Data.getParameter(YM_AMS, 0, -1) = (float)ams;
	*patch->InstrumentDef.Data.getParameter(YM_FMS, 0, -1) = (float)fms;

	if (ams > 0 || fms > 0)
		patch->InstrumentDef.LFOEnable = true;


	for( int i = 0; i < 4; i++ )
	{
		unsigned char mul = data[index];
		index++;
		*patch->InstrumentDef.Data.getParameter(YM_MUL, 0, i) = (float)mul;

		unsigned char dt = data[index];
		index++;
		*patch->InstrumentDef.Data.getParameter(YM_DT, 0, i) = (float)dtRevTable[dt];

		unsigned char tl = data[index];
		index++;
		*patch->InstrumentDef.Data.getParameter(YM_TL, 0, i) = 127 - (float)tl;

		unsigned char rs = data[index];
		index++;
		*patch->InstrumentDef.Data.getParameter(YM_KS, 0, i) = (float)rs;

		unsigned char ar = data[index];
		index++;
		*patch->InstrumentDef.Data.getParameter(YM_AR, 0, i) = 31 - (float)ar;

		unsigned char dram = data[index];
		index++;

		unsigned char dr1 = dram & 31;
		unsigned char am = (dram >> 7) & 1;
		*patch->InstrumentDef.Data.getParameter(YM_DR, 0, i) = 31 - (float)dr1;
		*patch->InstrumentDef.Data.getParameter(YM_AM, 0, i) = (float)am;

		unsigned char dr2 = data[index];
		index++;
		*patch->InstrumentDef.Data.getParameter(YM_SR, 0, i) = (float)dr2;

		unsigned char rr = data[index];
		index++;
		*patch->InstrumentDef.Data.getParameter(YM_RR, 0, i) = 15 - (float)rr;

		unsigned char dl1 = data[index];
		index++;
		*patch->InstrumentDef.Data.getParameter(YM_SL, 0, i) = 15 - (float)dl1;

		unsigned char ssg = data[index];
		switch(ssg)

		{

		}


		index++;
		*patch->InstrumentDef.Data.getParameter(YM_SSG, 0, i) = (float)GennyLoaders::ssgRegToGenny(ssg);
	}

	patch->InstrumentDef.Type = GIType::FM;
	patch->InstrumentDef.Ch3Special = false;
}

GennyData GennyLoaders::saveVGI(GennyPatch* patch)
{
	GennyData dat;
	unsigned char val = 0;
	int index = 0;

	dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_ALG, 0, -1));
	dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_FB, 0, -1));

	unsigned char ams = patch->InstrumentDef.Data.getParameterChar(YM_AMS, 0, -1);
	unsigned char fms = patch->InstrumentDef.Data.getParameterChar(YM_FMS, 0, -1);
	unsigned char amfm = (ams << 4) | fms;
	dat.writeByte(amfm);

	for( int i = 0; i < 4; i++ )
	{
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_MUL, 0, i));
		dat.writeByte(dtTable[patch->InstrumentDef.Data.getParameterChar(YM_DT, 0, i)]);
		dat.writeByte(127 - patch->InstrumentDef.Data.getParameterChar(YM_TL, 0, i));
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_KS, 0, i));
		dat.writeByte(31 - patch->InstrumentDef.Data.getParameterChar(YM_AR, 0, i));

		unsigned char dr1 = 31 - patch->InstrumentDef.Data.getParameterChar(YM_DR, 0, i);
		unsigned char am = patch->InstrumentDef.Data.getParameterChar(YM_AM, 0, i);
		unsigned char dram = (am << 7) | dr1;
		dat.writeByte(dram);
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_SR, 0, i));
		dat.writeByte(15 - patch->InstrumentDef.Data.getParameterChar(YM_RR, 0, i));
		dat.writeByte(15 - patch->InstrumentDef.Data.getParameterChar(YM_SL, 0, i));
		dat.writeByte(GennyLoaders::ssgGennyToReg(patch->InstrumentDef.Data.getParameterChar(YM_SSG, 0, i)));
	}

	return dat;
}

const int kGENVersion1 = 4392586;
const int kGENVersion2 = 4392587;
const int kGENVersion3 = 4392588;
const int kGENVersion4 = 4392589;
const int kGENVersionCurrent = kGENVersion4;
const int kGENVersionMax= 4393588;
const int kNewData01 = 5252535;
bool isVersionNumber(int pNumber)
{
	return pNumber >= kGENVersion1
		&& pNumber < kGENVersionMax;
}

void GennyLoaders::loadGEN(GennyPatch* patch, GennyData* loadData)
{
	if (patch == nullptr)
		throw std::exception("Cannot load information into null patch!");

	int version = loadData->readInt();
	if (!isVersionNumber(version))
	{
		loadData->dataPos -= 4;
		version = 0;
	}
	else if (version > kGENVersionCurrent)
	{
		int msgboxID = MessageBoxA(
			NULL,
			"The file you are opening was saved with a newer version of GENNY. This plugin is backwards compatible, but not forwards compatible- so to load this patch correctly you must install the latest version.",
			"GEN Version Too New",
			MB_ICONEXCLAMATION
		);
	}

	patch->Name = loadData->readString();

	loadGDAC(patch, loadData);

	bool dac = false;
	bool fm = false;
	if (version < kGENVersion3)
	{
		dac = loadData->readByte();
		fm = loadData->readByte();
	}

	*patch->InstrumentDef.Data.getParameter(YM_ALG, 0, -1) = (float)loadData->readByte();
	*patch->InstrumentDef.Data.getParameter(YM_FB, 0, -1) = (float)loadData->readByte();

	float ams = (float)loadData->readByte();
	float fms = (float)loadData->readByte();

	*patch->InstrumentDef.Data.getParameter(YM_AMS, 0, -1) = ams;
	*patch->InstrumentDef.Data.getParameter(YM_FMS, 0, -1) = fms;

	if (ams > 0 || fms > 0)
		patch->InstrumentDef.LFOEnable = true;
	else
		patch->InstrumentDef.LFOEnable = false;

	if (version != 0 && version < kGENVersion2)
	{
		/**patch->InstrumentDef.Data.getParameter(YM_LFO, 0, -1) = */(float)loadData->readByte(); //This is a global setting, it never should have been saved
		/**patch->InstrumentDef.Data.getParameter(YM_LFO_EN, 0, -1) = */(float)loadData->readByte(); //This is a global setting, it never should have been saved
	}

	if (version < kGENVersion3)
	{
		//Old versions had some parameters inversed to the way that they're displayed, this is now handled chipside
		for (int i = 0; i < 4; i++)
		{
			*patch->InstrumentDef.Data.getParameter(YM_MUL, 0, i) = (float)loadData->readByte();
			*patch->InstrumentDef.Data.getParameter(YM_DT, 0, i) = (float)loadData->readByte();
			*patch->InstrumentDef.Data.getParameter(YM_TL, 0, i) = 127 - (float)loadData->readByte();
			*patch->InstrumentDef.Data.getParameter(YM_KS, 0, i) = (float)loadData->readByte();
			*patch->InstrumentDef.Data.getParameter(YM_AR, 0, i) = 31 - (float)loadData->readByte();
			*patch->InstrumentDef.Data.getParameter(YM_DR, 0, i) = 31 - (float)loadData->readByte();
			*patch->InstrumentDef.Data.getParameter(YM_AM, 0, i) = (float)loadData->readByte();
			*patch->InstrumentDef.Data.getParameter(YM_SR, 0, i) = (float)loadData->readByte();
			*patch->InstrumentDef.Data.getParameter(YM_RR, 0, i) = 15 - (float)loadData->readByte();
			*patch->InstrumentDef.Data.getParameter(YM_SL, 0, i) = 15 - (float)loadData->readByte();
			*patch->InstrumentDef.Data.getParameter(YM_SSG, 0, i) = (float)loadData->readByte();
		}
	}
	else
	{
		for (int i = 0; i < 4; i++)
		{
			*patch->InstrumentDef.Data.getParameter(YM_MUL, 0, i) = (float)loadData->readByte();
			*patch->InstrumentDef.Data.getParameter(YM_DT, 0, i) = (float)loadData->readByte();
			*patch->InstrumentDef.Data.getParameter(YM_TL, 0, i) = (float)loadData->readByte();
			*patch->InstrumentDef.Data.getParameter(YM_KS, 0, i) = (float)loadData->readByte();
			*patch->InstrumentDef.Data.getParameter(YM_AR, 0, i) = (float)loadData->readByte();
			*patch->InstrumentDef.Data.getParameter(YM_DR, 0, i) = (float)loadData->readByte();
			*patch->InstrumentDef.Data.getParameter(YM_AM, 0, i) = (float)loadData->readByte();
			*patch->InstrumentDef.Data.getParameter(YM_SR, 0, i) = (float)loadData->readByte();
			*patch->InstrumentDef.Data.getParameter(YM_RR, 0, i) = (float)loadData->readByte();
			*patch->InstrumentDef.Data.getParameter(YM_SL, 0, i) = (float)loadData->readByte();
			*patch->InstrumentDef.Data.getParameter(YM_SSG, 0, i) = (float)loadData->readByte();
		}
	}

	*patch->InstrumentDef.Data.getParameter(SN_DT, 0, -1) = (float)loadData->readByte();

	float periodic = (float)loadData->readByte();
	*patch->InstrumentDef.Data.getParameter(SN_PERIODIC, 0, -1) = periodic;
	*patch->InstrumentDef.Data.getParameter(SN_SR, 0, -1) = (float)loadData->readByte();
	*patch->InstrumentDef.Data.getParameter(YM_DRUMTL, 0, 3) = (float)loadData->readByte();

	if (version != 0)
	{
		patch->InstrumentDef.OperatorVelocity[0] = (bool)loadData->readByte();
		patch->InstrumentDef.OperatorVelocity[1] = (bool)loadData->readByte();
		patch->InstrumentDef.OperatorVelocity[2] = (bool)loadData->readByte();
		patch->InstrumentDef.OperatorVelocity[3] = (bool)loadData->readByte();
	}

	bool melodicMode = false;


	if (version < kGENVersion3)
	{
		if (loadData->dataPos + 4 < loadData->size)
		{
			int read = loadData->readInt();
			if (read == kNewData01)
			{
				melodicMode = loadData->readByte() != 0;
			}
			else
				loadData->dataPos -= 4;
		}
	}


	if (version >= kGENVersion3)
	{
		patch->InstrumentDef.Ch3Special = loadData->readByte() > 0;
		patch->InstrumentDef.snMelodicEnable = loadData->readByte() > 0;

		if (version >= kGENVersion4)
			patch->InstrumentDef.LFOEnable = loadData->readByte() > 0;

		for (int i = 0; i < 4; i++)
		{
			patch->InstrumentDef.OperatorEnable[i] = loadData->readByte() > 0;
			patch->InstrumentDef.OperatorTranspose[i] = loadData->readByte();
			patch->InstrumentDef.OperatorOctave[i] = loadData->readByte();
			patch->InstrumentDef.OperatorDetune[i] = loadData->readByte();
		}

		patch->InstrumentDef.Type = (GIType::GIType)loadData->readByte();
	}
	else
	{
		patch->InstrumentDef.Ch3Special = false;
		patch->InstrumentDef.snMelodicEnable = false;
	}

	if (version < kGENVersion3)
	{
		if (dac)
		{
			patch->InstrumentDef.Type = GIType::DAC;

			patch->InstrumentDef.Channels[0] = false;
			patch->InstrumentDef.Channels[1] = false;
			patch->InstrumentDef.Channels[2] = false;
			patch->InstrumentDef.Channels[3] = false;
			patch->InstrumentDef.Channels[4] = false;
			patch->InstrumentDef.Channels[5] = true;
			patch->InstrumentDef.Channels[6] = false;
			patch->InstrumentDef.Channels[7] = false;
			patch->InstrumentDef.Channels[8] = false;
			patch->InstrumentDef.Channels[9] = false;

		}
		else if (fm)
		{
			patch->InstrumentDef.Type = GIType::FM;

			patch->InstrumentDef.Channels[0] = true;
			patch->InstrumentDef.Channels[1] = true;
			patch->InstrumentDef.Channels[2] = true;
			patch->InstrumentDef.Channels[3] = true;
			patch->InstrumentDef.Channels[4] = true;
			patch->InstrumentDef.Channels[5] = true;
			patch->InstrumentDef.Channels[6] = false;
			patch->InstrumentDef.Channels[7] = false;
			patch->InstrumentDef.Channels[8] = false;
			patch->InstrumentDef.Channels[9] = false;
		}
		else if (melodicMode)
		{
			patch->InstrumentDef.Type = GIType::SNSPECIAL;

			patch->InstrumentDef.Channels[0] = false;
			patch->InstrumentDef.Channels[1] = false;
			patch->InstrumentDef.Channels[2] = false;
			patch->InstrumentDef.Channels[3] = false;
			patch->InstrumentDef.Channels[4] = false;
			patch->InstrumentDef.Channels[5] = false;
			patch->InstrumentDef.Channels[6] = false;
			patch->InstrumentDef.Channels[7] = false;
			patch->InstrumentDef.Channels[8] = true;
			patch->InstrumentDef.Channels[9] = true;
			patch->InstrumentDef.snMelodicEnable = true;
		}
		else if (periodic > 0.0f)
		{
			patch->InstrumentDef.Type = GIType::SNDRUM;

			patch->InstrumentDef.Channels[0] = false;
			patch->InstrumentDef.Channels[1] = false;
			patch->InstrumentDef.Channels[2] = false;
			patch->InstrumentDef.Channels[3] = false;
			patch->InstrumentDef.Channels[4] = false;
			patch->InstrumentDef.Channels[5] = false;
			patch->InstrumentDef.Channels[6] = false;
			patch->InstrumentDef.Channels[7] = false;
			patch->InstrumentDef.Channels[8] = false;
			patch->InstrumentDef.Channels[9] = true;
		}
		else
		{
			patch->InstrumentDef.Type = GIType::SN;

			patch->InstrumentDef.Channels[0] = false;
			patch->InstrumentDef.Channels[1] = false;
			patch->InstrumentDef.Channels[2] = false;
			patch->InstrumentDef.Channels[3] = false;
			patch->InstrumentDef.Channels[4] = false;
			patch->InstrumentDef.Channels[5] = false;
			patch->InstrumentDef.Channels[6] = true;
			patch->InstrumentDef.Channels[7] = true;
			patch->InstrumentDef.Channels[8] = true;
			patch->InstrumentDef.Channels[9] = false;
		}
	}
}

GennyData GennyLoaders::saveGEN(GennyPatch* patch)
{
	GennyData dat;
	unsigned char val = 0;
	int index = 0;

	dat.writeInt(kGENVersionCurrent);
	dat.writeString(patch->Name);

	//Save drum sample information
	saveGDAC(patch, &dat);

	//Write YM2612 information
	dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_ALG, 0, -1));
	dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_FB, 0, -1));
	dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_AMS, 0, -1));
	dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_FMS, 0, -1));
	//dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_LFO, 0, -1));
	//dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_LFO_EN, 0, -1));

	for( int i = 0; i < 4; i++ )
	{
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_MUL, 0, i));
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_DT, 0, i));
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_TL, 0, i));
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_KS, 0, i));
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_AR, 0, i));
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_DR, 0, i));
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_AM, 0, i));
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_SR, 0, i));
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_RR, 0, i));
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_SL, 0, i));
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_SSG, 0, i));
	}

	dat.writeByte(patch->InstrumentDef.Data.getParameterChar(SN_DT, 0, -1));
	dat.writeByte(patch->InstrumentDef.Data.getParameterChar(SN_PERIODIC, 0, -1));
	dat.writeByte(patch->InstrumentDef.Data.getParameterChar(SN_SR, 0, -1));
	dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_DRUMTL, 0, 3));


	dat.writeByte(patch->InstrumentDef.OperatorVelocity[0]);
	dat.writeByte(patch->InstrumentDef.OperatorVelocity[1]);
	dat.writeByte(patch->InstrumentDef.OperatorVelocity[2]);
	dat.writeByte(patch->InstrumentDef.OperatorVelocity[3]);



	dat.writeByte(patch->InstrumentDef.Ch3Special ? 1 : 0);
	dat.writeByte(patch->InstrumentDef.snMelodicEnable ? 1 : 0);
	dat.writeByte(patch->InstrumentDef.LFOEnable ? 1 : 0);

	for (int i = 0; i < 4; i++)
	{
		dat.writeByte(patch->InstrumentDef.OperatorEnable[i] ? 1 : 0);
		dat.writeByte(patch->InstrumentDef.OperatorTranspose[i]);
		dat.writeByte(patch->InstrumentDef.OperatorOctave[i]);
		dat.writeByte(patch->InstrumentDef.OperatorDetune[i]);
	}

	dat.writeByte(patch->InstrumentDef.Type);
	return dat;
}

void GennyLoaders::loadDPACK(GennyPatch* patch, GennyData& info, int* streamPos)
{
	int startPos = info.dataPos;

	while(1)
	{
		char dat = info.readByte();
		if(dat == '*')
		{
			unsigned char num = info.readByte();
			int size = info.readInt();
			
			WaveData* wave = new WaveData(&info.data[info.dataPos], size);
			patch->InstrumentDef.Drumset.mapDrum(num, wave);
			info.dataPos += size;
		}
		else if(dat == '@')
		{
			break;
		}
	}
	*streamPos += info.dataPos; 
	patch->InstrumentDef.Data._operators[0][YM_TL] = 127;
	patch->InstrumentDef.Data._operators[1][YM_TL] = 127;
	patch->InstrumentDef.Data._operators[2][YM_TL] = 127;
	patch->InstrumentDef.Data._operators[3][YM_TL] = 64;
	
	patch->InstrumentDef.Type = GIType::DAC;
}

const long gdacVersion1 = 241493957835;
const long gdacVersion2 = 241493957836;
const long gdacVersion3 = 241493957837;
const long gdacCurrentVersion = gdacVersion3;

const long gdacVersionMax = 241493958836;//do not touch
bool isGDACVersion(long val)
{
	return val >= gdacVersion1 && val <= gdacVersionMax;
}

void GennyLoaders::loadGDAC(GennyPatch* patch, GennyData* loadData)
{
	unsigned char val = 0;
	int index = 0;

	long version = loadData->readLong();
	int numDrums = 56;
	bool oldVersion = false;
	if (!isGDACVersion(version)) //Old Version
	{
		loadData->dataPos -= 8;
		numDrums = 55;
		oldVersion = true;
	}
	else if (version > gdacCurrentVersion)
	{
		int msgboxID = MessageBoxA(
			NULL,
			"The file you are opening was saved with a newer version of GENNY. This plugin is backwards compatible, but not forwards compatible- so to load this drumset correctly you must install the latest version.",
			"GDAC Version Too New",
			MB_ICONEXCLAMATION
		);
	}

	for(int i = 36; i < numDrums; i++)
	{
		char has = loadData->readByte();
		if(has == 1)
		{	
			int sampleStart = 0;
			int sampleEnd = 0;
			bool sampleLoop = false;
			if (oldVersion == false)
			{
				sampleStart = loadData->readInt();
				sampleEnd = loadData->readInt();
				sampleLoop = loadData->readByte();
			}

			int sampleRate = loadData->readInt();
			int sampleSize = loadData->readInt();
			char* sampleData = new char[sampleSize];			
			memcpy(sampleData, &((char*)loadData->data)[loadData->dataPos], sampleSize);
			loadData->dataPos += sampleSize;

			WaveData* wave = new WaveData(sampleData, sampleSize);
			wave->sampleRate = sampleRate;
			wave->originalSampleRate = sampleRate;

			if (oldVersion == false)
			{
				wave->_startSample = wave->_originalStartSample = sampleStart;
				wave->_endSample = wave->_originalEndSample = sampleEnd;
				wave->loop = sampleLoop;
			}

			if (version >= gdacVersion3)
			{
				int resampledRate = loadData->readInt();
				if (resampledRate != sampleRate)
				{
					int resampleSize = loadData->readInt();
					unsigned char* resampleData = new unsigned char[resampleSize];
					memcpy(resampleData, &((unsigned char*)loadData->data)[loadData->dataPos], resampleSize);
					loadData->dataPos += resampleSize;

					wave->reprocess(resampledRate, resampleData, resampleSize);
				}
			}

			if (version >= gdacVersion2)
			{
				wave->panLeft = loadData->readByte() == 1;
				wave->panRight = loadData->readByte() == 1;
				wave->_amp = loadData->readByte();
				wave->_pitch = loadData->readShort();
			}

			WaveData* drum = patch->InstrumentDef.Drumset.getDrum(i);
			if(drum != nullptr)
				delete drum;

			patch->InstrumentDef.Drumset.mapDrum(i, wave);
		}
	}
	
	patch->InstrumentDef.Type = GIType::DAC;
	*patch->InstrumentDef.Data.getParameter(YM_DRUMTL, 0, 3) = 100;
	//patch->InstrumentDef.Channels[0] = false;
	//patch->InstrumentDef.Channels[1] = false;
	//patch->InstrumentDef.Channels[2] = false;
	//patch->InstrumentDef.Channels[3] = false;
	//patch->InstrumentDef.Channels[4] = false;
	//patch->InstrumentDef.Channels[5] = true;
	//patch->InstrumentDef.Channels[6] = false;
	//patch->InstrumentDef.Channels[7] = false;
	//patch->InstrumentDef.Channels[8] = false;
	//patch->InstrumentDef.Channels[9] = false;
}

GennyData* GennyLoaders::saveGDAC(GennyPatch* patch, GennyData* data)
{
	if(data == nullptr)
		data = new GennyData();


	data->writeLong(gdacCurrentVersion);
	for(int i = 36; i < 56; i++)
	{
		WaveData* wave = patch->InstrumentDef.Drumset.getDrum(i);
		if(wave != nullptr)
		{
			data->writeByte(1);
			data->writeInt(wave->_originalStartSample);
			data->writeInt(wave->_originalEndSample);
			data->writeByte(wave->loop);

			data->writeInt(wave->originalSampleRate);
			data->writeInt(wave->originalDataSize);
			data->writeBytes((char*)wave->originalAudioData, wave->originalDataSize);

			data->writeInt(wave->sampleRate);
			if (wave->sampleRate != wave->originalSampleRate)
			{
				data->writeInt(wave->size);
				data->writeBytes((char*)wave->audioData, wave->size);
			}

			data->writeByte(wave->panLeft);
			data->writeByte(wave->panRight);
			data->writeByte(wave->_amp);
			data->writeShort(wave->_pitch);
		}
		else
			data->writeByte(0);
	}

	return data;
}
