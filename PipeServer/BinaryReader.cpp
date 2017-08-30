#include "BinaryReader.hpp"

BinaryReader::BinaryReader(Stream& _input)
	: input(_input),
	  buffer{ 0 }
{

}
//---------------------------------------------------------------------------
bool BinaryReader::ReadBoolean()
{
	FillBuffer(1);
	return buffer[0] != 0;
}
//---------------------------------------------------------------------------
uint8_t BinaryReader::ReadByte() const
{
	const int b = input.ReadByte();
	if (b == -1)
	{
		throw IOException();
	}
	return static_cast<uint8_t>(b);
}
//---------------------------------------------------------------------------
std::vector<uint8_t> BinaryReader::ReadBytes(int count) const
{
	std::vector<uint8_t> result(count);
	result.resize(count);

	int numRead = 0;
	do
	{
		const int n = input.Read(result.data(), numRead, count);
		if (n == 0)
		{
			break;
		}
		numRead += n;
		count -= n;
	} while (count > 0);

	if (numRead != count)
	{
		result = std::vector<uint8_t>(std::begin(result), std::begin(result) + numRead);
	}

	return result;
}
//---------------------------------------------------------------------------
short BinaryReader::ReadInt16()
{
	FillBuffer(2);
	return static_cast<short>(buffer[0] | buffer[1] << 8);
}
//---------------------------------------------------------------------------
unsigned short BinaryReader::ReadUInt16()
{
	FillBuffer(2);
	return static_cast<unsigned short>(buffer[0] | buffer[1] << 8);
}
//---------------------------------------------------------------------------
int BinaryReader::ReadInt32()
{
	FillBuffer(4);
	return static_cast<int>(buffer[0] | buffer[1] << 8 | buffer[2] << 16 | buffer[3] << 24);
}
//---------------------------------------------------------------------------
unsigned int BinaryReader::ReadUInt32()
{
	FillBuffer(4);
	return static_cast<unsigned int>(buffer[0] | buffer[1] << 8 | buffer[2] << 16 | buffer[3] << 24);
}
//---------------------------------------------------------------------------
long long BinaryReader::ReadInt64()
{
	FillBuffer(8);
	const auto lo = static_cast<unsigned int>(buffer[0] | buffer[1] << 8 | buffer[2] << 16 | buffer[3] << 24);
	const auto hi = static_cast<unsigned int>(buffer[4] | buffer[5] << 8 | buffer[6] << 16 | buffer[7] << 24);
	return static_cast<long long>(static_cast<unsigned long long>(hi)) << 32 | lo;
}
//---------------------------------------------------------------------------
unsigned long long BinaryReader::ReadUInt64()
{
	FillBuffer(8);
	const auto lo = static_cast<unsigned int>(buffer[0] | buffer[1] << 8 | buffer[2] << 16 | buffer[3] << 24);
	const auto hi = static_cast<unsigned int>(buffer[4] | buffer[5] << 8 | buffer[6] << 16 | buffer[7] << 24);
	return static_cast<unsigned long long>(hi) << 32 | lo;
}
//---------------------------------------------------------------------------
void* BinaryReader::ReadIntPtr()
{
#ifdef _WIN64
	return reinterpret_cast<void*>(ReadUInt64());
#else
	return reinterpret_cast<void*>(ReadUInt32());
#endif
}
//---------------------------------------------------------------------------
float BinaryReader::ReadSingle()
{
	auto tmp = ReadUInt32();
	return *reinterpret_cast<float*>(&tmp);
}
//---------------------------------------------------------------------------
double BinaryReader::ReadDouble()
{
	auto tmp = ReadUInt64();
	return *reinterpret_cast<double*>(&tmp);
}
//---------------------------------------------------------------------------
std::wstring BinaryReader::ReadString() const
{
	const auto MaxCharBytesSize = 128;

	const auto byteLength = Read7BitEncodedInt();
	if (byteLength < 0)
	{
		throw IOException();
	}

	if (byteLength == 0)
	{
		return std::wstring();
	}

	std::wstring tmp;

	int currPos = 0;
	uint8_t charBytes[MaxCharBytesSize];
	do
	{
		const auto readLength = (byteLength - currPos) > MaxCharBytesSize ? MaxCharBytesSize : byteLength - currPos;

		const auto n = input.Read(charBytes, 0, readLength);
		if (n == 0)
		{
			throw IOException();
		}

		tmp.append(reinterpret_cast<const std::wstring::value_type*>(charBytes), readLength / sizeof(std::wstring::value_type));

		currPos += n;
	} while (currPos < byteLength);

	return tmp;
}
//---------------------------------------------------------------------------
void BinaryReader::FillBuffer(int numBytes)
{
	int bytesRead = 0;
	int n = 0;

	if (numBytes == 1)
	{
		n = input.ReadByte();
		if (n == -1)
		{
			throw IOException();
		}
		buffer[0] = static_cast<uint8_t>(n);
		return;
	}

	do
	{
		n = input.Read(buffer, bytesRead, numBytes - bytesRead);
		if (n == 0)
		{
			throw IOException();
		}
		bytesRead += n;
	} while (bytesRead < numBytes);
}
//---------------------------------------------------------------------------
int BinaryReader::Read7BitEncodedInt() const
{
	// Read out an int32 7 bits at a time. The high bit of the
	// byte when on means to continue reading more bytes.
	int count = 0;
	int shift = 0;
	uint8_t b;
	do
	{
		if (shift == 5 * 7)
		{
			throw IOException();
		}

		b = ReadByte();
		count |= (b & 0x7F) << shift;
		shift += 7;
	} while ((b & 0x80) != 0);

	return count;
}
//---------------------------------------------------------------------------
