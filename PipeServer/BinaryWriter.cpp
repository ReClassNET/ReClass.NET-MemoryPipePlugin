#include "BinaryWriter.hpp"

BinaryWriter::BinaryWriter(Stream& _output)
	: output(_output),
	  buffer{ 0 }
{

}
//---------------------------------------------------------------------------
void BinaryWriter::Write(bool value)
{
	buffer[0] = static_cast<uint8_t>(value ? 1 : 0);
	output.Write(buffer, 0, 1);
}
//---------------------------------------------------------------------------
void BinaryWriter::Write(uint8_t value) const
{
	output.WriteByte(value);
}
//---------------------------------------------------------------------------
void BinaryWriter::Write(const uint8_t* buffer, int offset, int length) const
{
	output.Write(buffer, 0, length);
}
//---------------------------------------------------------------------------
void BinaryWriter::Write(short value)
{
	buffer[0] = static_cast<uint8_t>(value);
	buffer[1] = static_cast<uint8_t>(value >> 8);
	output.Write(buffer, 0, 2);
}
//---------------------------------------------------------------------------
void BinaryWriter::Write(unsigned short value)
{
	buffer[0] = static_cast<uint8_t>(value);
	buffer[1] = static_cast<uint8_t>(value >> 8);
	output.Write(buffer, 0, 2);
}
//---------------------------------------------------------------------------
void BinaryWriter::Write(int value)
{
	buffer[0] = static_cast<uint8_t>(value);
	buffer[1] = static_cast<uint8_t>(value >> 8);
	buffer[2] = static_cast<uint8_t>(value >> 16);
	buffer[3] = static_cast<uint8_t>(value >> 24);
	output.Write(buffer, 0, 4);
}
//---------------------------------------------------------------------------
void BinaryWriter::Write(unsigned int value)
{
	buffer[0] = static_cast<uint8_t>(value);
	buffer[1] = static_cast<uint8_t>(value >> 8);
	buffer[2] = static_cast<uint8_t>(value >> 16);
	buffer[3] = static_cast<uint8_t>(value >> 24);
	output.Write(buffer, 0, 4);
}
//---------------------------------------------------------------------------
void BinaryWriter::Write(long long value)
{
	buffer[0] = static_cast<uint8_t>(value);
	buffer[1] = static_cast<uint8_t>(value >> 8);
	buffer[2] = static_cast<uint8_t>(value >> 16);
	buffer[3] = static_cast<uint8_t>(value >> 24);
	buffer[4] = static_cast<uint8_t>(value >> 32);
	buffer[5] = static_cast<uint8_t>(value >> 40);
	buffer[6] = static_cast<uint8_t>(value >> 48);
	buffer[7] = static_cast<uint8_t>(value >> 56);
	output.Write(buffer, 0, 8);
}
//---------------------------------------------------------------------------
void BinaryWriter::Write(unsigned long long value)
{
	buffer[0] = static_cast<uint8_t>(value);
	buffer[1] = static_cast<uint8_t>(value >> 8);
	buffer[2] = static_cast<uint8_t>(value >> 16);
	buffer[3] = static_cast<uint8_t>(value >> 24);
	buffer[4] = static_cast<uint8_t>(value >> 32);
	buffer[5] = static_cast<uint8_t>(value >> 40);
	buffer[6] = static_cast<uint8_t>(value >> 48);
	buffer[7] = static_cast<uint8_t>(value >> 56);
	output.Write(buffer, 0, 8);
}
//---------------------------------------------------------------------------
void BinaryWriter::Write(const void* value)
{
#ifdef _WIN64
	Write(reinterpret_cast<unsigned long long>(value));
#else
	Write(reinterpret_cast<unsigned int>(value));
#endif
}
//---------------------------------------------------------------------------
void BinaryWriter::Write(float value)
{
	const auto tmp = *reinterpret_cast<unsigned int*>(&value);
	buffer[0] = static_cast<uint8_t>(tmp);
	buffer[1] = static_cast<uint8_t>(tmp >> 8);
	buffer[2] = static_cast<uint8_t>(tmp >> 16);
	buffer[3] = static_cast<uint8_t>(tmp >> 24);
	output.Write(buffer, 0, 4);
}
//---------------------------------------------------------------------------
void BinaryWriter::Write(double value)
{
	const auto tmp = *reinterpret_cast<unsigned long long*>(&value);
	buffer[0] = static_cast<uint8_t>(tmp);
	buffer[1] = static_cast<uint8_t>(tmp >> 8);
	buffer[2] = static_cast<uint8_t>(tmp >> 16);
	buffer[3] = static_cast<uint8_t>(tmp >> 24);
	buffer[4] = static_cast<uint8_t>(tmp >> 32);
	buffer[5] = static_cast<uint8_t>(tmp >> 40);
	buffer[6] = static_cast<uint8_t>(tmp >> 48);
	buffer[7] = static_cast<uint8_t>(tmp >> 56);
	output.Write(buffer, 0, 8);
}
//---------------------------------------------------------------------------
void BinaryWriter::Write(const std::wstring& value) const
{
	const int byteLength = static_cast<int>(value.length()) * sizeof(std::wstring::value_type);
	Write7BitEncodedInt(byteLength);

	output.Write(reinterpret_cast<const uint8_t*>(value.data()), 0, byteLength);
}
//---------------------------------------------------------------------------
void BinaryWriter::Write7BitEncodedInt(int value) const
{
	// Write out an int32 7 bits at a time. The high bit of the byte,
	// when on, tells reader to continue reading more byte.
	auto v = static_cast<unsigned int>(value);
	while (v >= 0x80)
	{
		Write(static_cast<uint8_t>(v | 0x80));
		v >>= 7;
	}
	Write(static_cast<uint8_t>(v));
}
//---------------------------------------------------------------------------
