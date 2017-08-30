#pragma once

#include <string>

#include "Stream.hpp"

class BinaryWriter
{
public:
	BinaryWriter(Stream& output);

	void Write(bool value);

	void Write(uint8_t value) const;

	void Write(const uint8_t* buffer, int offset, int length) const;

	void Write(short value);

	void Write(unsigned short value);

	void Write(int value);

	void Write(unsigned int value);

	void Write(long long value);

	void Write(unsigned long long value);

	void Write(const void* value);

	void Write(float value);

	void Write(double value);

	void Write(const std::wstring& value) const;

private:
	void Write7BitEncodedInt(int value) const;

	Stream& output;
	uint8_t buffer[8];
};
