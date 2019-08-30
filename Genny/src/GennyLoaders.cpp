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

	HRSRC hRes = FindResourceW(hMod, MAKEINTRESOURCEW(name), format.c_str());
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
GennyPatch* GennyLoaders::loadTYI(const std::string& name, GennyData& info, bool del)
{
	char* data = info.data;

	GennyPatch* newPatch = new GennyPatch(name);
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

		*newPatch->InstrumentDef.Data.getParameter(YM_MUL, 0, i) = (float)mul;
		*newPatch->InstrumentDef.Data.getParameter(YM_DT, 0, i) = (float)dt;
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
		*newPatch->InstrumentDef.Data.getParameter(YM_TL, 0, i) = (float)tl;
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
		*newPatch->InstrumentDef.Data.getParameter(YM_KS, 0, i) = (float)rs;
		*newPatch->InstrumentDef.Data.getParameter(YM_AR, 0, i) = (float)ar;
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
		*newPatch->InstrumentDef.Data.getParameter(YM_AM, 0, i) = (float)am;
		*newPatch->InstrumentDef.Data.getParameter(YM_DR, 0, i) = (float)dr1;
	}

	//DR2
	for( int i = 0; i < 4; i++ )
	{
		val = data[index];
		index++;
		unsigned char dr2 = val;
		*newPatch->InstrumentDef.Data.getParameter(YM_SR, 0, i) = (float)dr2;
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
		*newPatch->InstrumentDef.Data.getParameter(YM_SL, 0, i) = (float)dl1;
		*newPatch->InstrumentDef.Data.getParameter(YM_RR, 0, i) = (float)rr;
	}

	//SSG-EG
	for( int i = 0; i < 4; i++ )
	{
		val = data[index];
		*newPatch->InstrumentDef.Data.getParameter(YM_SSG, 0, i) = (float)GennyLoaders::ssgRegToGenny(val);
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
	*newPatch->InstrumentDef.Data.getParameter(YM_FB, 0, -1) = (float)fb;
	*newPatch->InstrumentDef.Data.getParameter(YM_ALG, 0, -1) = (float)alg;

	val = data[index];
	index++;
	//AMS_FMS
	//------------------------------
	//  0   1   2   3   4   5   6   7
	// |L| |R|  |AMS|       |  FMS  |
	unsigned char ams = (val >> 4) & 3;
	unsigned char fms = val & 7;
	*newPatch->InstrumentDef.Data.getParameter(YM_AMS, 0, -1) = (float)ams;
	*newPatch->InstrumentDef.Data.getParameter(YM_FMS, 0, -1) = (float)fms;
	
	newPatch->InstrumentDef.Type = GIType::FM;

	if(del)
	{
		if(info.handle != 0)
			FreeResource(info.handle);
		else
			delete[] info.data;
	}

	return newPatch;
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
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_TL, 0, i));
	}

	for( int i = 0; i < 4; i++ )
	{

		unsigned char rs = patch->InstrumentDef.Data.getParameterChar(YM_KS, 0, i);
		unsigned char ar = patch->InstrumentDef.Data.getParameterChar(YM_AR, 0, i);
		unsigned char rsar = (rs << 6) | ar;
		dat.writeByte(rsar);
	}

	for( int i = 0; i < 4; i++ )
	{
		unsigned char am = patch->InstrumentDef.Data.getParameterChar(YM_AM, 0, i);
		unsigned char dr1 = patch->InstrumentDef.Data.getParameterChar(YM_DR, 0, i);
		unsigned char amdr = (am << 7) | dr1;
		dat.writeByte(amdr);
	}

	for( int i = 0; i < 4; i++ )
	{
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_SR, 0, i));
	}

	for( int i = 0; i < 4; i++ )
	{
		unsigned char dl1 = patch->InstrumentDef.Data.getParameterChar(YM_SL, 0, i);
		unsigned char rr = patch->InstrumentDef.Data.getParameterChar(YM_RR, 0, i);
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

GennyPatch* GennyLoaders::loadTFI(const std::string& name, GennyData& info)
{
	char* data = info.data;

	GennyPatch* newPatch = new GennyPatch(name);
	unsigned char val = 0;
	int index = 0;

	//FB_ALGORITHM
	//------------------------------
	unsigned char alg = data[index];
	index++;
	unsigned char fb = data[index];
	index++;
	*newPatch->InstrumentDef.Data.getParameter(YM_FB, 0, -1) = (float)fb;
	*newPatch->InstrumentDef.Data.getParameter(YM_ALG, 0, -1) = (float)alg;

	for( int i = 0; i < 4; i++ )
	{
		unsigned char mul = data[index];
		index++;
		*newPatch->InstrumentDef.Data.getParameter(YM_MUL, 0, i) = (float)mul;

		unsigned char dt = data[index];
		index++;
		*newPatch->InstrumentDef.Data.getParameter(YM_DT, 0, i) = (float)dtRevTable[dt];

		unsigned char tl = data[index];
		index++;
		*newPatch->InstrumentDef.Data.getParameter(YM_TL, 0, i) = (float)tl;

		unsigned char rs = data[index];
		index++;
		*newPatch->InstrumentDef.Data.getParameter(YM_KS, 0, i) = (float)rs;

		unsigned char ar = data[index];
		index++;
		*newPatch->InstrumentDef.Data.getParameter(YM_AR, 0, i) = (float)ar;

		unsigned char dr1 = data[index];
		index++;
		*newPatch->InstrumentDef.Data.getParameter(YM_DR, 0, i) = (float)dr1;

		unsigned char dr2 = data[index];
		index++;
		*newPatch->InstrumentDef.Data.getParameter(YM_SR, 0, i) = (float)dr2;

		unsigned char rr = data[index];
		index++;
		*newPatch->InstrumentDef.Data.getParameter(YM_RR, 0, i) = (float)rr;

		unsigned char dl1 = data[index];
		index++;
		*newPatch->InstrumentDef.Data.getParameter(YM_SL, 0, i) = (float)dl1;

		unsigned char ssg = data[index];
		index++;
		*newPatch->InstrumentDef.Data.getParameter(YM_SSG, 0, i) = (float)GennyLoaders::ssgRegToGenny(ssg);
	}

	if(info.handle != 0)
		FreeResource(info.handle);
	else
		delete[] info.data;
	
	newPatch->InstrumentDef.Type = GIType::FM;

	return newPatch;
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
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_TL, 0, i));
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_KS, 0, i));
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_AR, 0, i));
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_DR, 0, i));
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_SR, 0, i));
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_RR, 0, i));
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_SL, 0, i));
		dat.writeByte(GennyLoaders::ssgGennyToReg(patch->InstrumentDef.Data.getParameterChar(YM_SSG, 0, i)));
	}

	return dat;
}

GennyPatch* GennyLoaders::loadVGI(const std::string& name, const std::string& prefix, GennyData& info, bool del)
{
	char* data = info.data;

	GennyPatch* newPatch = new GennyPatch(name);
	newPatch->Prefix = prefix;
	unsigned char val = 0;
	int index = 0;

	unsigned char alg = data[index];
	index++;
	unsigned char fb = data[index];
	index++;
	*newPatch->InstrumentDef.Data.getParameter(YM_FB, 0, -1) = (float)fb;
	*newPatch->InstrumentDef.Data.getParameter(YM_ALG, 0, -1) = (float)alg;


	unsigned char amfm = data[index];
	index++;

	unsigned char ams = (amfm >> 4) & 3;
	unsigned char fms = amfm & 7;
	*newPatch->InstrumentDef.Data.getParameter(YM_AMS, 0, -1) = (float)ams;
	*newPatch->InstrumentDef.Data.getParameter(YM_FMS, 0, -1) = (float)fms;


	for( int i = 0; i < 4; i++ )
	{
		unsigned char mul = data[index];
		index++;
		*newPatch->InstrumentDef.Data.getParameter(YM_MUL, 0, i) = (float)mul;

		unsigned char dt = data[index];
		index++;
		*newPatch->InstrumentDef.Data.getParameter(YM_DT, 0, i) = (float)dtRevTable[dt];

		unsigned char tl = data[index];
		index++;
		*newPatch->InstrumentDef.Data.getParameter(YM_TL, 0, i) = (float)tl;

		unsigned char rs = data[index];
		index++;
		*newPatch->InstrumentDef.Data.getParameter(YM_KS, 0, i) = (float)rs;

		unsigned char ar = data[index];
		index++;
		*newPatch->InstrumentDef.Data.getParameter(YM_AR, 0, i) = (float)ar;

		unsigned char dram = data[index];
		index++;

		unsigned char dr1 = dram & 31;
		unsigned char am = (dram >> 7) & 1;
		*newPatch->InstrumentDef.Data.getParameter(YM_DR, 0, i) = (float)dr1;
		*newPatch->InstrumentDef.Data.getParameter(YM_AM, 0, i) = (float)am;

		unsigned char dr2 = data[index];
		index++;
		*newPatch->InstrumentDef.Data.getParameter(YM_SR, 0, i) = (float)dr2;

		unsigned char rr = data[index];
		index++;
		*newPatch->InstrumentDef.Data.getParameter(YM_RR, 0, i) = (float)rr;

		unsigned char dl1 = data[index];
		index++;
		*newPatch->InstrumentDef.Data.getParameter(YM_SL, 0, i) = (float)dl1;

		unsigned char ssg = data[index];
		switch(ssg)

		{

		}


		index++;
		*newPatch->InstrumentDef.Data.getParameter(YM_SSG, 0, i) = (float)GennyLoaders::ssgRegToGenny(ssg);
	}

	if(del)
	{
		if(info.handle != 0)
			FreeResource(info.handle);
		else
			delete[] info.data;
	}

	newPatch->InstrumentDef.Type = GIType::FM;
	return newPatch;
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
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_TL, 0, i));
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_KS, 0, i));
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_AR, 0, i));

		unsigned char dr1 = patch->InstrumentDef.Data.getParameterChar(YM_DR, 0, i);
		unsigned char am = patch->InstrumentDef.Data.getParameterChar(YM_AM, 0, i);
		unsigned char dram = (am << 7) | dr1;
		dat.writeByte(dram);
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_SR, 0, i));
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_RR, 0, i));
		dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_SL, 0, i));
		dat.writeByte(GennyLoaders::ssgGennyToReg(patch->InstrumentDef.Data.getParameterChar(YM_SSG, 0, i)));
	}

	return dat;
}

GennyPatch* GennyLoaders::loadGEN(const std::string& name, const std::string& prefix, GennyPatch* patch, GennyData* loadData, bool del)
{	
	GennyPatch* newPatch = patch;
	
	if(newPatch == nullptr)
		newPatch = new GennyPatch(name);

	newPatch->Prefix = prefix;

	newPatch->Name = loadData->readString();

	loadGDAC(newPatch, loadData, false);

	bool dac = loadData->readByte();
	bool fm = loadData->readByte();
	
	*newPatch->InstrumentDef.Data.getParameter(YM_ALG, 0, -1) = (float)loadData->readByte();
	*newPatch->InstrumentDef.Data.getParameter(YM_FB, 0, -1) = (float)loadData->readByte();
	*newPatch->InstrumentDef.Data.getParameter(YM_AMS, 0, -1) = (float)loadData->readByte();
	*newPatch->InstrumentDef.Data.getParameter(YM_FMS, 0, -1) = (float)loadData->readByte();

	for( int i = 0; i < 4; i++ )
	{
		*newPatch->InstrumentDef.Data.getParameter(YM_MUL, 0, i) = (float)loadData->readByte();
		*newPatch->InstrumentDef.Data.getParameter(YM_DT, 0, i) = (float)loadData->readByte();
		*newPatch->InstrumentDef.Data.getParameter(YM_TL, 0, i) = (float)loadData->readByte();
		*newPatch->InstrumentDef.Data.getParameter(YM_KS, 0, i) = (float)loadData->readByte();
		*newPatch->InstrumentDef.Data.getParameter(YM_AR, 0, i) = (float)loadData->readByte();
		*newPatch->InstrumentDef.Data.getParameter(YM_DR, 0, i) = (float)loadData->readByte();
		*newPatch->InstrumentDef.Data.getParameter(YM_AM, 0, i) = (float)loadData->readByte();
		*newPatch->InstrumentDef.Data.getParameter(YM_SR, 0, i) = (float)loadData->readByte();
		*newPatch->InstrumentDef.Data.getParameter(YM_RR, 0, i) = (float)loadData->readByte();
		*newPatch->InstrumentDef.Data.getParameter(YM_SL, 0, i) = (float)loadData->readByte();
		*newPatch->InstrumentDef.Data.getParameter(YM_SSG, 0, i) = (float)loadData->readByte();
	}

	*newPatch->InstrumentDef.Data.getParameter(SN_DT, 0, -1) = (float)loadData->readByte();
	
	float periodic = (float)loadData->readByte();
	*newPatch->InstrumentDef.Data.getParameter(SN_PERIODIC, 0, -1) = periodic;
	*newPatch->InstrumentDef.Data.getParameter(SN_SR, 0, -1) = (float)loadData->readByte();
	*newPatch->InstrumentDef.Data.getParameter(YM_DRUMTL, 0, 3) = (float)loadData->readByte();


	if(del)
	{
		if(loadData->handle != 0)
			FreeResource(loadData->handle); 
		else
			delete[] loadData->data;
	}
	
	if(dac)
		newPatch->InstrumentDef.Type = GIType::DAC;
	else if(fm)
		newPatch->InstrumentDef.Type = GIType::FM;
	else if(periodic > 0.0f)
		newPatch->InstrumentDef.Type = GIType::SNDRUM;
	else
		newPatch->InstrumentDef.Type = GIType::SN;

	return newPatch;
}

GennyData GennyLoaders::saveGEN(GennyPatch* patch)
{
	GennyData dat;
	unsigned char val = 0;
	int index = 0;

	//Save drum sample information
	dat.writeString(patch->Name);
	saveGDAC(patch, &dat);

	if(patch->InstrumentDef.Type == GIType::FM)
	{
		dat.writeByte(0);
		dat.writeByte(1);
	}
	else if(patch->InstrumentDef.Type == GIType::DAC)
	{
		dat.writeByte(1);
		dat.writeByte(1);
	}else
	{
		dat.writeByte(0);
		dat.writeByte(0);
	}

	//Write YM2612 information
	dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_ALG, 0, -1));
	dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_FB, 0, -1));
	dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_AMS, 0, -1));
	dat.writeByte(patch->InstrumentDef.Data.getParameterChar(YM_FMS, 0, -1));

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



	return dat;
}

GennyPatch* GennyLoaders::loadDPACK(const std::string& name, GennyData& info, int* streamPos)
{
	GennyPatch* newPatch = new GennyPatch(name);

	int startPos = info.dataPos;

	while(1)
	{
		char dat = info.readByte();
		if(dat == '*')
		{
			unsigned char num = info.readByte();
			int size = info.readInt();
			
			WaveData* wave = new WaveData(&info.data[info.dataPos], size);
			newPatch->InstrumentDef.Drumset.mapDrum(num, wave);
			info.dataPos += size;
		}
		else if(dat == '@')
		{
			break;
		}
	}
	*streamPos += info.dataPos; 
	newPatch->InstrumentDef.Data._operators[0][YM_TL] = 127;
	newPatch->InstrumentDef.Data._operators[1][YM_TL] = 127;
	newPatch->InstrumentDef.Data._operators[2][YM_TL] = 127;
	newPatch->InstrumentDef.Data._operators[3][YM_TL] = 64;
	
	newPatch->InstrumentDef.Type = GIType::DAC;

	return newPatch;
}

void GennyLoaders::loadGDAC(GennyPatch* patch, GennyData* loadData, bool del)
{
	unsigned char val = 0;
	int index = 0;

	for(int i = 36; i < 55; i++)
	{
		char has = loadData->readByte();
		if(has == 1)
		{	
			int sampleRate = loadData->readInt();
			int sampleSize = loadData->readInt();
			char* sampleData = new char[sampleSize];			
			memcpy(sampleData, &((char*)loadData->data)[loadData->dataPos], sampleSize);
			loadData->dataPos += sampleSize;

			WaveData* wave = new WaveData(sampleData, sampleSize);
			wave->sampleRate = sampleRate;

			WaveData* drum = patch->InstrumentDef.Drumset.getDrum(i);
			if(drum != nullptr)
			{
				if(drum->audioData != nullptr)
					delete drum->audioData;

				delete drum;
			}

			patch->InstrumentDef.Drumset.mapDrum(i, wave);
		}
	}

	if(del)
	{
		if(loadData->handle != 0)
			FreeResource(loadData->handle);
		else
			delete[] loadData->data;
	}

	
	patch->InstrumentDef.Type = GIType::DAC;
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

	for(int i = 36; i < 55; i++)
	{
		WaveData* wave = patch->InstrumentDef.Drumset.getDrum(i);
		if(wave != nullptr)
		{
			data->writeByte(1);
			data->writeInt(wave->sampleRate);
			data->writeInt(wave->size);
			data->writeBytes((char*)wave->audioData, wave->size);
		}
		else
			data->writeByte(0);
	}

	return data;
}
