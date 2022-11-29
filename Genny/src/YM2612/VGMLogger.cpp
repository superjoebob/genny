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
	_totalSamples(0),
	_drumStreamPosition(0),
	_vst(nullptr)
{

}

void VGMLogger::initialize(YM2612* ymChip, SN76489Chip* snChip)
{
	_ymChip = ymChip;
	_snChip = snChip;
}

void VGMLogger::startLogging(GennyVST* vst, std::string file)
{
	_vst = vst;

	_dacStream = std::ostringstream();
	_commandStream = std::ostringstream();

	_loopPosition = 0;
	_loopStartSample = 0;
	_sampleCounter = 0;
	_totalSamples = 0;
	_drumStreamPosition = 0;
	_dataStream.open(file.c_str(),std::ios::out | std::ios::binary | std::ios::trunc);

	_dataStream.write("Vgm ", 4);
	_dataStream.write("EOF ", 4);

	//Replace with end of file offset, size of file - 4
	_dataStream << (unsigned char)0x50;
	_dataStream << (unsigned char)0x01;
	_dataStream << (unsigned char)0x00;
	_dataStream << (unsigned char)0x00;

	int clock = _snChip->_clock;
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

	clock = _ymChip->_clock;
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

	////Write drum sample data
	//bool hasDrums = false;
	//int drumSizePos = 0;
	//int drumSamplePosition = 0;
	//GennyPatch* patch0 = static_cast<GennyPatch*>(vst->getPatch(0));
	//for(int i = 0; i < 16; i++)
	//{
	//	if(patch0->Instruments[i] != -1)
	//	{
	//		GennyPatch* patch = static_cast<GennyPatch*>(vst->getPatch(patch0->Instruments[i]));
	//		if(patch->InstrumentDef.Type == GIType::DAC)
	//		{
	//			if(hasDrums == false)
	//			{
	//				_dataStream << (unsigned char)0x67;
	//				_dataStream << (unsigned char)0x66;
	//				_dataStream << (unsigned char)0x00;

	//				//Fill in drum size later, when we know it.
	//				drumSizePos = _dataStream.tellg();
	//				_dataStream.write((const char*)&zero, 4);

	//				hasDrums = true;
	//			}

	//			DrumSet* drums = &patch->InstrumentDef.Drumset;


	//			for (int i = 36; i < 55; i++)
	//			{
	//				char has = 0;
	//				WaveData* wave = drums->getDrum(i);
	//				if (wave != nullptr)
	//				{
	//					wave->streamStartPos = -1;
	//					//float vol = _ymChip->_sampleVolume;
	//					//vol *= (wave->_amp / 100.0f) * 2.0f;

	//					//unsigned char* newdat = new unsigned char[wave->size];

	//					////For logging, sample volume levels need to be baked
	//					//for (int samp = 0; samp < wave->size; samp++)
	//					//{
	//					//	newdat[samp] = min(255, max(0, (int)((((float)wave->audioData[samp] - 128.0f) * vol) + 0.5f) + 128));
	//					//}

	//					//wave->streamStartPos = drumSamplePosition;
	//					//_dataStream.write((const char*)newdat, wave->size);

	//					//delete[] newdat;
	//					//drumSamplePosition += wave->size;
	//				}
	//			}
	//		}
	//	}
	//}
	//
	////Write drums size
	//if(hasDrums)
	//{
	//	int pos = _dataStream.tellg();
	//	_dataStream.seekg(drumSizePos, std::ios::beg);
	//	_dataStream.write((const char*)&drumSamplePosition, 4);
	//	_dataStream.seekg(pos, std::ios::beg);
	//}

	//Reset all DAC stream positions
	GennyPatch* patch0 = static_cast<GennyPatch*>(vst->getPatch(0));
	for(int i = 0; i < 16; i++)
	{
		if(patch0->Instruments[i] != -1)
		{
			GennyPatch* patch = static_cast<GennyPatch*>(vst->getPatch(patch0->Instruments[i]));
			if(patch->InstrumentDef.Type == GIType::DAC)
			{
				DrumSet* drums = &patch->InstrumentDef.Drumset;
				for (int i = 36; i < 55; i++)
				{
					char has = 0;
					WaveData* wave = drums->getDrum(i);
					if (wave != nullptr)
					{
						wave->streamStartPos = -1;
						memset(wave->streamStarts, -1, kNumDACVelos * sizeof(int));
					}
				}
			}
		}
	}

	_logging = true;
}

void VGMLogger::prepareDrum(WaveData* drum, float velocity)
{
	int streamStartIndex = drum->getStreamStartIndex(velocity);
	if (drum->streamStarts[streamStartIndex] < 0)
	{
		float vol = velocity;
		vol *= (drum->_amp / 100.0f) * 2.0f;

		unsigned char* newdat = new unsigned char[drum->size];

		//For logging, sample volume levels need to be baked
		for (int samp = 0; samp < drum->size; samp++)
		{
			newdat[samp] = min(255, max(0, (int)((((float)drum->audioData[samp] - 128.0f) * vol) + 0.5f) + 128));
		}

		drum->streamStarts[streamStartIndex] = _drumStreamPosition;
		_dacStream.write((const char*)newdat, drum->size);

		delete[] newdat;
		_drumStreamPosition += drum->size;
	}
}

void VGMLogger::finishLogging()
{
	//Write drumstream
	if (_drumStreamPosition > 0)
	{
		_dataStream << (unsigned char)0x67;
		_dataStream << (unsigned char)0x66;
		_dataStream << (unsigned char)0x00;
		_dataStream.write((const char*)&_drumStreamPosition, 4); //Write drum size


		std::string str = _dacStream.str();
		_dataStream << _dacStream.str();
	}
	

	//Write command stream
	_dataStream << _commandStream.str();
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
		_commandStream << (unsigned char)0x61;
		_commandStream.write((const char*)&_sampleCounter, 2);
		_sampleCounter = 0;
	}
}

void VGMLogger::logWriteSN76489(unsigned char data)
{
	if(_sampleCounter > 0)
	{
		_commandStream << (unsigned char)0x61;
		_commandStream.write((const char*)&_sampleCounter, 2);
		_sampleCounter = 0;
	}

	_commandStream << (unsigned char)0x50;
	_commandStream << data;
}

void VGMLogger::logWriteYM2612(int port, unsigned char reg, unsigned char data)
{
	if(_sampleCounter > 0)
	{
		_commandStream << (unsigned char)0x61;
		_commandStream.write((const char*)&_sampleCounter, 2);
		_sampleCounter = 0;
	}

	if(port == 0)
	{
		_commandStream << (unsigned char)0x52;
		_commandStream << reg;
		_commandStream << data;
	}
	else
	{
		_commandStream << (unsigned char)0x53;
		_commandStream << reg;
		_commandStream << data;
	}
}

void VGMLogger::seekDACSample(int seek)
{
	_commandStream << (unsigned char)0xe0;
	_commandStream.write((const char*)&seek, 4);
}

void VGMLogger::logDACSample()
{
	//if (_sampleCounter < 16)
	//{
	//	_dataStream << ((unsigned char)(0x8 << 4) | (unsigned char)_sampleCounter);
	//	_sampleCounter = 0;
	//}
	//else
	//{
		resetSampleCounter();
		_commandStream << (unsigned char)0x80;
	//}
}

void VGMLogger::resetSampleCounter()
{
	if(_sampleCounter > 0)
	{
		_commandStream << (unsigned char)0x61;
		_commandStream.write((const char*)&_sampleCounter, 2);
	}
	_sampleCounter = 0;
}

void VGMLogger::writeLoopPoint()
{
	_loopPosition = ((int)_commandStream.tellp()) - 0x1C;
	_loopStartSample = _totalSamples;
}