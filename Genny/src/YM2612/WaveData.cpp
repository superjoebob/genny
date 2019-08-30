#include "WaveData.h"
#include "GennyLoaders.h"
#include "resampler\libresample.h"
#include "resampler\Fir_Resampler.h"
#include <math.h> 
#include <stdio.h>
#include "filters\source\include\DspFilters\ChebyshevI.h"


static const int kFormatHeader1 = 'FFIR';
static const int kFormatHeader2 = 'EVAW';
static const int kListHeader = 'tsil';
static const int kDataHeader = 'atad';



WaveData::WaveData(GennyData* data)
	: size(-1)
	,sampleRate(-1)
	,bitRate(-1)
	,audioData(nullptr)
	,audioPosition(0)
	,waitTime(0.0f)
	,streamStartPos(0)
	,valid(true)
	,flash(false)	
	,startSample(0)
    ,endSample(0)
    ,loopSampleStart(0)
	,loopSampleEnd(0)
	,panLeft(true)
	,panRight(true)
	,loop(false)
	,normalize(false)
	,release(false)
{    
	sampleName = "Drum Sample";
	int chunkID = data->readInt();
	if(chunkID == kFormatHeader1)
	{
		int dataSize = data->readInt();
		int format = data->readInt();
		if(format == kFormatHeader2)
		{
			int subChunk1ID = data->readInt();
			int subChunk1Size = data->readInt();
			int prevDataPos = data->dataPos;

			unsigned short audioFormat = data->readUShort();
			if(audioFormat != 1)
			{
				valid = false;
				return;
			}

			unsigned short numChannels = data->readUShort();
			sampleRate = data->readInt();
			int byteRate = data->readInt();
			unsigned short blockAlign = data->readUShort();
			unsigned short bitRate = data->readUShort();

			data->dataPos = prevDataPos;
			data->dataPos += subChunk1Size;
			while(data->dataPos < dataSize + 8)
			{		
				int subChunkID = data->readInt();
				int chunkSize = data->readInt();

				//Only process the DATA header for now.
				if(subChunkID == kDataHeader)
				{
					bool deleteData = false;
					unsigned char* sampleData = nullptr;
					if(bitRate != 8)
					{
						int divisor = bitRate / 8;
						int newSize = chunkSize / divisor;

						int oldPos = data->dataPos;
						double* dubs = new double[chunkSize / 2];
						for(int i = 0; i < chunkSize / 2; i++)
						{
							short sample = data->readShort();
							float realSamp = ((sample / 32767.0f) + 1.0f) / 2.0f;
							dubs[i] = realSamp;
						}
						data->dataPos = oldPos;

						deleteData = true;
						sampleData = new unsigned char[newSize];
						for(int i = 0; i < newSize; i++)
						{
							sampleData[i] = dubs[i] * 255;
						}
						delete[] dubs;
						size = newSize;
					}
					else
					{
						sampleData = (unsigned char*)&data->data[data->dataPos];
						size = chunkSize;
					}

					if(numChannels != 1)
					{
						for(int i = 0; i < size; i += 2)
						{
							unsigned char samp = (sampleData[i] + sampleData[i + 1]) / 2;
							sampleData[i / 2] = samp;
						}
						size /= 2;
					}

					SetData(sampleData, size);
					data->dataPos += chunkSize;

					if(deleteData)
						delete[] sampleData;
				}
				else
				{
					//Skip headers we don't care about
					data->dataPos += chunkSize;
				}
			}
		}
	}
}

WaveData::WaveData(char* data, int sz)
	: size(sz)
	, sampleRate(11025)
	, bitRate(8)
	, audioData(nullptr)
	, audioPosition(0)
	, waitTime(0.0f)
	, streamStartPos(0)
	,valid(true)
	,flash(false)
	,startSample(0)
	,endSample(0)
	,loopSampleStart(0)
	,loopSampleEnd(0)
	,panLeft(true)
	,panRight(true)
	,loop(false)
	,normalize(false)
{
	sampleName = "Drum Sample";
	SetData((unsigned char*)data, sz);
}


void WaveData::SetData(unsigned char* data, int sz)
{
	int startPos = 0;
	int endPos = sz;
	size = endPos - startPos;
	audioData = new unsigned char[size];

	startSample = 0;
	endSample = size;

	memcpy(audioData, &data[startPos], size);
}

WaveData* WaveData::GetChunk(int start, int end)
{
	WaveData* newWaveData = new WaveData((char*)audioData, size);
	newWaveData->startSample = start;
	newWaveData->endSample = end;

	newWaveData->sampleName = sampleName;
	newWaveData->bitRate = bitRate;
	newWaveData->sampleRate = sampleRate;

	return newWaveData;
}


WaveData::~WaveData()
{
	if(audioData != nullptr)
		delete[] audioData;
}