#pragma once

#include <string>

#include "Stream.hpp"

class BinaryWriter
{
public:
	BinaryWriter(Stream& output);

	void Write(bool value);

	void Write(uint8_t value);

	void Write(const uint8_t* buffer, int offset, int length);

	void Write(short value);

	void Write(unsigned short value);

	void Write(int value);

	void Write(unsigned int value);

	void Write(long long value);

	void Write(unsigned long long value);

	void Write(const void* value);

	void Write(float value);

	void Write(double value);

	void Write(const std::wstring& value);

private:
	void Write7BitEncodedInt(int value);

	Stream& output;
	uint8_t buffer[8];
};
