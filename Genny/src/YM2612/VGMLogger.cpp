#include "VGMLogger.h"
#include "SN76489Chip.h"
#include "YM2612.h"
#include "GennyVST.h"
#include "Genny2612.h"
#include "intrin.h"

VGMLogger::VGMLogger() :
	_ymChip(nullptr),
	_snChip(nullptr),
	_sampleCounter(0),
	_logging(false),
	_totalSamples(0)
{

}

void VGMLogger::initialize(YM2612* ymChip, SN76489Chip* snChip)
{
	_ymChip = ymChip;
	_snChip = snChip;
}

void VGMLogger::startLogging(GennyVST* vst, std::string file)
{
	_loopPosition = 0;
	_loopStartSample = 0;
	_sampleCounter = 0;
	_totalSamples = 0;
	_dataStream.open(file.c_str(),std::ios::out | std::ios::binary | std::ios::trunc);

	_dataStream.write("Vgm ", 4);
	_dataStream.write("EOF ", 4);

	//Replace with end of file offset, size of file - 4
	_dataStream << (unsigned char)0x50;
	_dataStream << (unsigned char)0x01;
	_dataStream << (unsigned char)0x00;
	_dataStream << (unsigned char)0x00;

	int clock = SN76489_NTSC;
	//SN76489 clock
	_dataStream.write((const char*)&clock, 4);

	int zero = 0;

	//YM2413 clock
	_dataStream.write((const char*)&zero, 4);

	//GD3 offset
	_dataStream.write((const char*)&zero, 4);

	//total samples, equal to total wait values in file
	_dataStream.write((const char*)&zero, 4);

	//loop offset and loop samples
	_dataStream.write((const char*)&zero, 4);
	_dataStream.write((const char*)&zero, 4);

	int rate = 60;
	//rate scaling
	_dataStream.write((const char*)&rate, 4);

	short pattern = 0x0009;
	//noise feedback pattern
	_dataStream.write((const char*)&pattern, 2);
	//shift register width
	_dataStream << (unsigned char)16;
	//sn flags
	_dataStream << (unsigned char)0;

	clock = YM2612_NTSC;
	//YM2612 clock
	_dataStream.write((const char*)&clock, 4);

	//YM2151 clock
	_dataStream.write((const char*)&zero, 4);

	int offset = 0x0000000c;
	//VGM data offset
	_dataStream.write((const char*)&offset, 4);

	//Reserved
	_dataStream.write((const char*)&zero, 4);
	_dataStream.write((const char*)&zero, 4);

	//Write drum sample data
	bool hasDrums = false;
	int drumSizePos = 0;
	int drumSamplePosition = 0;
	GennyPatch* patch0 = static_cast<GennyPatch*>(vst->getPatch(0));
	for(int i = 0; i < 16; i++)
	{
		if(patch0->Instruments[i] != -1)
		{
			GennyPatch* patch = static_cast<GennyPatch*>(vst->getPatch(patch0->Instruments[i]));
			if(patch->InstrumentDef.Type == GIType::DAC)
			{
				if(hasDrums == false)
				{
					_dataStream << (unsigned char)0x67;
					_dataStream << (unsigned char)0x66;
					_dataStream << (unsigned char)0x00;

					//Fill in drum size later, when we know it.
					drumSizePos = _dataStream.tellg();
					_dataStream.write((const char*)&zero, 4);

					hasDrums = true;
				}

				DrumSet* drums = &patch->InstrumentDef.Drumset;
				for(int iDrum = 60; iDrum < 75; iDrum++)
				{
					WaveData* drum = drums->getDrum(iDrum);
					if(drum != nullptr)
					{
						drum->streamStartPos = drumSamplePosition;
						_dataStream.write((const char*)drum->audioData, drum->size);
						drumSamplePosition += drum->size;
					}
				}
			}
		}
	}
	
	//Write drums size
	if(hasDrums)
	{
		int pos = _dataStream.tellg();
		_dataStream.seekg(drumSizePos, std::ios::beg);
		_dataStream.write((const char*)&drumSamplePosition, 4);
		_dataStream.seekg(pos, std::ios::beg);
	}

	_logging = true;
}

void VGMLogger::finishLogging()
{
	_dataStream << (unsigned char)0x66;
	long end;
	_dataStream.seekg (0, std::ios::end);
	end = _dataStream.tellg();
	end -= 4;

	_dataStream.seekg( 4, std::ios::beg );
	_dataStream.write((const char*)&end, 4);

	_dataStream.seekg( 0x18, std::ios::beg );
	_dataStream.write((const char*)&_totalSamples, 4);

	_dataStream.seekg( 0x1C, std::ios::beg );
	_dataStream.write((const char*)&_loopPosition, 4);

	_dataStream.seekg( 0x20, std::ios::beg );
	_dataStream.write((const char*)&_totalSamples - _loopStartSample, 4);


	_dataStream.close();
	_logging = false;
}

void VGMLogger::logSample()
{
	_sampleCounter += 1;
	_totalSamples += 1;
	if(_sampleCounter == 65535)
	{
		_dataStream << (unsigned char)0x61;
		_dataStream.write((const char*)&_sampleCounter, 2);
		_sampleCounter = 0;
	}
}

void VGMLogger::logWriteSN76489(unsigned char data)
{
	if(_sampleCounter > 0)
	{
		_dataStream << (unsigned char)0x61;
		_dataStream.write((const char*)&_sampleCounter, 2);
		_sampleCounter = 0;
	}

	_dataStream << (unsigned char)0x50;
	_dataStream << data;
}

void VGMLogger::logWriteYM2612(int port, unsigned char reg, unsigned char data)
{
	if(_sampleCounter > 0)
	{
		_dataStream << (unsigned char)0x61;
		_dataStream.write((const char*)&_sampleCounter, 2);
		_sampleCounter = 0;
	}

	if(port == 0)
	{
		_dataStream << (unsigned char)0x52;
		_dataStream << reg;
		_dataStream << data;
	}
	else
	{
		_dataStream << (unsigned char)0x53;
		_dataStream << reg;
		_dataStream << data;
	}
}

void VGMLogger::seekDACSample(int seek)
{
	_dataStream << (unsigned char)0xe0;
	_dataStream.write((const char*)&seek, 4);
}

void VGMLogger::logDACSample()
{
	_dataStream << (unsigned char)0x80;
	resetSampleCounter();
}

void VGMLogger::resetSampleCounter()
{
	if(_sampleCounter > 0)
	{
		_dataStream << (unsigned char)0x61;
		_dataStream.write((const char*)&_sampleCounter, 2);
		_sampleCounter = 0;
	}
}

void VGMLogger::writeLoopPoint()
{
	_loopPosition = ((int)_dataStream.tellg()) - 0x1C;
	_loopStartSample = _totalSamples;
}