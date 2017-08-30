#pragma once

#include <cstdint>

class Stream
{
public:
	virtual ~Stream() = default;
	//---------------------------------------------------------------------------
	virtual int Read(uint8_t* buffer, int offset, int count) = 0;
	//---------------------------------------------------------------------------
	virtual int ReadByte()
	{
		uint8_t oneByteArray[1];
		const int r = Read(oneByteArray, 0, 1);
		if (r == 0)
		{
			return -1;
		}
		return oneByteArray[0];
	}
	//---------------------------------------------------------------------------
	virtual void Write(const uint8_t* buffer, int offset, int count) = 0;
	//---------------------------------------------------------------------------
	virtual void WriteByte(uint8_t value)
	{
		uint8_t oneByteArray[1] = { value };
		Write(oneByteArray, 0, 1);
	}
	//---------------------------------------------------------------------------
};
