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

unsigned char* WaveData::Resample(unsigned char* source, int sourceLength, int sourceSampleRate, int targetSampleRate, int& newSize)
{
	Resampler r;
	float sampInc = sourceSampleRate / (float)targetSampleRate;
	r.Fir_Resampler_initialize((sourceLength / (sampInc)) * 4.0f);
	r.Fir_Resampler_time_ratio((float)sourceSampleRate / targetSampleRate, 0.955);
	int firBufferPos = 0;
	for (int i = 0; i < sourceLength; i++)
	{
		r.Fir_Resampler_buffer()[firBufferPos] = (int)(((source[i] / 255.0f) * 65535) - 32767);
		r.Fir_Resampler_buffer()[firBufferPos + 1] = (int)(((source[i] / 255.0f) * 65535) - 32767);
		firBufferPos += 2;
	}
	r.Fir_Resampler_write(sourceLength * 2);

	int read = r.Fir_Resampler_avail();
	int* sampleData = new int[(long)(read * 2L)];
	int trueRead = r.Fir_Resampler_read((sample_t*)sampleData, read);

	unsigned char* newData = new unsigned char[read];
	for (int i = 0; i < read; i++)
	{ 
		newData[i] = (((sampleData[i * 2] / 32767.0f)) * 127) + 127;
	}

	delete[] sampleData;

	r.Fir_Resampler_shutdown();
	newSize = read;
	return newData;

	//unsigned char* sampleData = nullptr;

	//float sampInc = sourceSampleRate / (float)targetSampleRate;
	////if (sampInc < 1)
	////	sampInc = 1;

	//int newSize = sourceLength / (sampInc);
	//sampleData = new unsigned char[newSize];
	//int idx = 0;
	//float sampbuildup = 0;
	//for (int i = 0; i < sourceLength; i)
	//{
	//	if (idx >= newSize)
	//		break;

	//	while (sampbuildup < 1.0f)
	//	{
	//		sampleData[idx] = source[i];
	//		sampbuildup += sampInc;
	//		idx++;
	//	}
	//	i += (int)sampbuildup;
	//	sampbuildup = 0;
	//}

	//lastResampleRatio = targetSampleRate / (float)sourceSampleRate;
	//return sampleData;
}

WaveData* WaveData::getProcessed(int dacSamplerate)
{
	if (_processed == nullptr || _dirty)
		reprocess(dacSamplerate);
	 
	return _processed;
}

void WaveData::reprocess(int dacSamplerate, unsigned char* presampledData, int presampledDataSize)
{
	if (originalSampleRate <= 0)
		originalSampleRate = sampleRate;

	float pitchOffset = pow(2.0, ((_pitch / 50) / 12.0));
	dacSamplerate /= pitchOffset;

	if (sampleRate == dacSamplerate)
		return;

	if (dacSamplerate == originalSampleRate)
	{
		if (audioData != originalAudioData && audioData != nullptr)
			delete[] audioData;

		size = originalDataSize;
		sampleRate = originalSampleRate;
		audioData = originalAudioData;

		setStartSample(_originalStartSample);
		setEndSample(_originalEndSample);
		setLoopSampleStart(_originalLoopSampleStart);
		setLoopSampleEnd(_originalLoopSampleEnd);

		return; //No resampling neccessary
	}

	if (presampledData != nullptr && presampledDataSize > 0)
	{
		if (audioData != originalAudioData && audioData != nullptr)
			delete[] audioData;

		size = presampledDataSize;
		audioData = presampledData;
	}
	else
	{
		//Do some costly resampling
		Resampler r;
		float samplerateRatio = originalSampleRate / (float)dacSamplerate;
		r.Fir_Resampler_initialize(max(originalDataSize, (int)(originalDataSize / samplerateRatio)) * 2);
		r.Fir_Resampler_time_ratio(samplerateRatio, 0.995);
		int firBufferPos = 0;
		for (int i = 0; i < originalDataSize; i++)
		{
			r.Fir_Resampler_buffer()[firBufferPos] = (int)(((originalAudioData[i] / 255.0f) * 65535) - 32767);
			r.Fir_Resampler_buffer()[firBufferPos + 1] = (int)(((originalAudioData[i] / 255.0f) * 65535) - 32767);
			firBufferPos += 2;
		}
		r.Fir_Resampler_write(originalDataSize * 2);

		int read = r.Fir_Resampler_avail();
		int* sampleData = new int[(long)(read * 2L)];
		int trueRead = r.Fir_Resampler_read((sample_t*)sampleData, read);

		if (audioData != originalAudioData && audioData != nullptr)
			delete[] audioData;

		size = read;

		audioData = new unsigned char[read];
		for (int i = 0; i < read; i++)
		{
			audioData[i] = ((((sampleData[i * 2] / 32767.0f)) * 127) + 128);
		}

		delete[] sampleData;

		r.Fir_Resampler_shutdown();
	}

	sampleRate = dacSamplerate;
	setStartSample(_originalStartSample);
	setEndSample(_originalEndSample);
	setLoopSampleStart(_originalLoopSampleStart);
	setLoopSampleEnd(_originalLoopSampleEnd);
}


WaveData::WaveData(GennyData* data)
	: size(-1)
	, sampleRate(-1)
	, bitRate(-1)
	, audioData(nullptr)
	, audioPosition(0)
	, waitTime(0.0f)
	, streamStartPos(0)
	, valid(true)
	, flash(false)
	, _startSample(0)
	, _endSample(0)
	, _loopSampleStart(0)
	, _loopSampleEnd(0)
	, _originalStartSample(0)
	, _originalEndSample(0)
	, _originalLoopSampleStart(0)
	, _originalLoopSampleEnd(0)
	, panLeft(true)
	, panRight(true)
	, loop(false)
	, normalize(false)
	, release(false)
	, fadeSamples(-1)
	, _amp(50)
	, _pitch(0)
	, _dirty(false)
	, _processed(nullptr)
	, originalSampleRate(-1)
	, originalAudioData(nullptr)
	, originalDataSize(-1)
{    
	memset(streamStarts, -1, kNumDACVelos * sizeof(int));


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

						float sampInc = sampleRate / 11025.0f;
						if (sampInc < 1)
							sampInc = 1;

						int loopSize = newSize;
						newSize = newSize / (sampInc);
						sampleData = new unsigned char[newSize];
						int idx = 0;
						for(int i = 0; i < loopSize; i)
						{
							if (idx >= newSize)
								break;

							sampleData[idx] = dubs[i] * 255;
							i += (int)sampInc;
							idx++;
						}

						if (sampleRate > 11025)
							sampleRate = 11025;

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
					originalSampleRate = sampleRate;

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

void WaveData::setStartSample(int val)
{
	_originalStartSample = val;
	_startSample = min((int)(_originalStartSample / (originalSampleRate / (float)sampleRate)), size);
}
void WaveData::setEndSample(int val)
{
	_originalEndSample = val;
	_endSample = min((int)(_originalEndSample / (originalSampleRate / (float)sampleRate)), size);
}
void WaveData::setLoopSampleStart(int val)
{
	_originalLoopSampleStart = val;
	_loopSampleStart = min((int)(_originalLoopSampleStart / (originalSampleRate / (float)sampleRate)), size);
}
void WaveData::setLoopSampleEnd(int val)
{
	_originalLoopSampleEnd = val;
	_loopSampleEnd = min((int)(_originalLoopSampleEnd / (originalSampleRate / (float)sampleRate)), size);
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
	,_startSample(0)
	,_endSample(0)
	,_loopSampleStart(0)
	,_loopSampleEnd(0)
	,_originalStartSample(0)
	,_originalEndSample(0)
	,_originalLoopSampleStart(0)
	,_originalLoopSampleEnd(0)
	,panLeft(true)
	,panRight(true)
	,loop(false)
	,normalize(false)
	,_pitch(0)
	,_amp(50)
	,_dirty(false)
	, _processed(nullptr)
	, originalSampleRate(11025)
	, originalAudioData(nullptr)
	, originalDataSize(sz)
{
	memset(streamStarts, -1, kNumDACVelos * sizeof(int));
	sampleName = "Drum Sample";
	if(data != nullptr)
		SetData((unsigned char*)data, sz);
}

int WaveData::getStreamStartIndex(float velocity)
{
	int streamStartIndex = ((int)velocity / 8.0f) * (kNumDACVelos - 1);
	if (streamStartIndex < 0)
		streamStartIndex = 0;
	if (streamStartIndex >= kNumDACVelos)
		streamStartIndex = kNumDACVelos - 1;

	return streamStartIndex;
}

int WaveData::getStreamStart(float velocity)
{
	return streamStarts[getStreamStartIndex(velocity)];
}

void WaveData::SetData(unsigned char* data, int sz)
{
	int startPos = 0;
	int endPos = sz;
	originalDataSize = size = endPos - startPos;
	audioData = originalAudioData = new unsigned char[size];

	_startSample = _originalStartSample = 0;
	_endSample = _originalEndSample = size;

	memcpy(audioData, &data[startPos], size);
}

WaveData* WaveData::GetChunk(int start, int end)
{
	WaveData* newWaveData = new WaveData((char*)audioData, size);
	newWaveData->_startSample = _originalStartSample = start;
	newWaveData->_endSample = _originalEndSample = end;

	newWaveData->sampleName = sampleName;
	newWaveData->bitRate = bitRate;
	newWaveData->sampleRate = sampleRate;

	return newWaveData;
}


WaveData::~WaveData()
{
	if (audioData != nullptr && audioData != originalAudioData)
		delete[] audioData;

	if (originalAudioData != nullptr)
		delete[] originalAudioData;

	if (_processed != nullptr)
		delete _processed;
}