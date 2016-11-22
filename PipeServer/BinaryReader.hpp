#pragma once

#include <vector>

#include "Stream.hpp"
#include "Exceptions.hpp"

class BinaryReader
{
public:
	BinaryReader(Stream& input);

	bool ReadBoolean();

	uint8_t ReadByte();

	std::vector<uint8_t> ReadBytes(int count);

	short ReadInt16();

	unsigned short ReadUInt16();

	int ReadInt32();

	unsigned int ReadUInt32();

	long long ReadInt64();

	unsigned long long ReadUInt64();

	void* ReadIntPtr();

	float ReadSingle();

	double ReadDouble();

	std::wstring ReadString();

private:
	void FillBuffer(int numBytes);

	int Read7BitEncodedInt();

	Stream& input;
	uint8_t buffer[8];
};
