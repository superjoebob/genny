#pragma once
#include "../resource.h"
#include <stdio.h>
#include <windows.h>
#include <string>

struct GennyData
{
	const static long version1 = 1127443255;
	const static long currentVersion = version1; //CHANGE THIS FOR NEW VERSIONS!
	GennyData() : handle(0), dataPos(0), data(nullptr), size(0), fullSize(0), createdData(false)
	{

	}

	//~GennyData()
	//{
	//	if (data != nullptr && createdData)
	//		delete[] data;
	//}

	void readVersion()
	{
		version = readLong();
	}

	void writeVersion()
	{
		writeLong(currentVersion);
	}

	unsigned char readByte()
	{
		unsigned char dat = data[dataPos];
		dataPos += 1;
		return dat;
	}

	int readShort()
	{
		short dat = *((short*)&data[dataPos]);
		dataPos += 2;
		return dat;
	}

	unsigned short readUShort()
	{
		unsigned short dat = *((unsigned short*)&data[dataPos]);
		dataPos += 2;
		return dat;
	}


	int readInt()
	{
		int dat = *((int*)&data[dataPos]);
		dataPos += 4;
		return dat;
	}

	float readFloat()
	{
		float dat = *((float*)&data[dataPos]);
		dataPos += 4;
		return dat;
	}

	long readLong()
	{
		long dat = *((long*)&data[dataPos]);
		dataPos += 8;
		return dat;
	}

	double readDouble()
	{
		double dat = *((double*)&data[dataPos]);
		dataPos += 8;
		return dat;
	}

	std::string readString()
	{
		int size = readInt();
		
		char* str = new char[size];
		memcpy(str, &data[dataPos], size);
		dataPos += size;
		std::string s = str;
		delete[] str;
		return s;
	}

	void writeByte(char b)
	{
		if(size - dataPos < 1)
			resize(size + 1);

		data[dataPos] = b;
		dataPos += 1;
	}	

	void writeBytes(char* b, int bSize)
	{
		if(size - dataPos < bSize)
			resize(size + bSize);

		memcpy(&data[dataPos], b, bSize);
		dataPos += bSize;
	}	

	void writeShort(short b)
	{
		if (size - dataPos < 2)
			resize(size + 2);

		data[dataPos] = ((char*)&b)[0];
		data[dataPos + 1] = ((char*)&b)[1];

		dataPos += 2;
	}

	void writeInt(int b)
	{
		if(size - dataPos < 4)
			resize(size + 4);

		data[dataPos] = ((char*)&b)[0];
		data[dataPos + 1] = ((char*)&b)[1];
		data[dataPos + 2] = ((char*)&b)[2];
		data[dataPos + 3] = ((char*)&b)[3];

		dataPos += 4;
	}

	void writeFloat(float b)
	{
		if (size - dataPos < 4)
			resize(size + 4);

		data[dataPos] = ((char*)&b)[0];
		data[dataPos + 1] = ((char*)&b)[1];
		data[dataPos + 2] = ((char*)&b)[2];
		data[dataPos + 3] = ((char*)&b)[3];

		dataPos += 4;
	}

	void writeLong(long b)
	{
		if (size - dataPos < 8)
			resize(size + 8);

		data[dataPos] = ((char*)&b)[0];
		data[dataPos + 1] = ((char*)&b)[1];
		data[dataPos + 2] = ((char*)&b)[2];
		data[dataPos + 3] = ((char*)&b)[3];
		data[dataPos + 4] = ((char*)&b)[4];
		data[dataPos + 5] = ((char*)&b)[5];
		data[dataPos + 6] = ((char*)&b)[6];
		data[dataPos + 7] = ((char*)&b)[7];

		dataPos += 8;
	}

	void writeDouble(double b)
	{
		if (size - dataPos < 8)
			resize(size + 8);

		data[dataPos] = ((char*)&b)[0];
		data[dataPos + 1] = ((char*)&b)[1];
		data[dataPos + 2] = ((char*)&b)[2];
		data[dataPos + 3] = ((char*)&b)[3];
		data[dataPos + 4] = ((char*)&b)[4];
		data[dataPos + 5] = ((char*)&b)[5];
		data[dataPos + 6] = ((char*)&b)[6];
		data[dataPos + 7] = ((char*)&b)[7];

		dataPos += 8;
	}

	void writeString(std::string str)
	{
		if(size - dataPos < ((int)str.length() + 1) + 4)
			resize(size + ((int)str.length() + 1) + 4);

		writeInt(str.length() + 1);

		
		char * cstr = new char [(int)str.length()+1];
		std::strcpy (cstr, str.c_str());

		memcpy(&data[dataPos], cstr, (int)str.length() + 1);
		delete[] cstr;

		dataPos += (int)str.length() + 1;
	}

	void resize(int s)
	{
		if(s > size)
		{
			if(data == nullptr)
			{
				createdData = true;
				data = new char[s];
				size = s;
			}
			else
			{
				createdData = true;
				char* newData = new char[s * 2];

				memcpy(newData, data, size);
				size = s;
				fullSize = s * 2;
				delete[] data;
				data = newData;
			}
		}
	}

	HGLOBAL handle;
	int size;
	int fullSize;
	char* data;

	int dataPos;
	long version;
	bool createdData = false;
};