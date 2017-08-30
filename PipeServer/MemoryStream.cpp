#include "MemoryStream.hpp"
#include "Exceptions.hpp"

MemoryStream::MemoryStream()
	: MemoryStream(0)
{

}
//---------------------------------------------------------------------------
MemoryStream::MemoryStream(int _capacity)
	: buffer(_capacity),
	  length(0),
	  capacity(_capacity),
	  position(0),
	  isOpen(true),
	  isExpandable(true)
{

}
//---------------------------------------------------------------------------
MemoryStream::MemoryStream(const std::vector<uint8_t>& _buffer)
	: buffer(_buffer),
	  length(static_cast<int>(_buffer.size())),
	  capacity(static_cast<int>(_buffer.size())),
	  position(0),
	  isOpen(true),
	  isExpandable(false)
{

}
//---------------------------------------------------------------------------
MemoryStream::~MemoryStream()
{
	isOpen = false;
}
//---------------------------------------------------------------------------
int MemoryStream::GetCapacity() const
{
	if (!isOpen)
	{
		throw IOException();
	}
	return capacity;
}
//---------------------------------------------------------------------------
void MemoryStream::SetCapacity(int value)
{
	if (!isOpen)
	{
		throw IOException();
	}
	if (!isExpandable && (value != capacity))
	{
		throw IOException();
	}

	if (isExpandable && value != capacity)
	{
		if (value > 0)
		{
			buffer.resize(value);
		}
		else
		{
			buffer.clear();
		}

		capacity = value;
	}
}
//---------------------------------------------------------------------------
int MemoryStream::GetLength() const
{
	if (!isOpen)
	{
		throw IOException();
	}
	return static_cast<int>(buffer.size());
}
//---------------------------------------------------------------------------
int MemoryStream::GetPosition() const
{
	if (!isOpen)
	{
		throw IOException();
	}
	return position;
}
//---------------------------------------------------------------------------
void MemoryStream::SetPosition(int value)
{
	if (!isOpen)
	{
		throw IOException();
	}
	position = value;
}
//---------------------------------------------------------------------------
int MemoryStream::Read(uint8_t* _buffer, int offset, int count)
{
	if (!isOpen)
	{
		throw IOException();
	}

	int n = length - position;
	if (n > count)
	{
		n = count;
	}
	if (n <= 0)
	{
		return 0;
	}

	int byteCount = n;
	while (--byteCount >= 0)
	{
		_buffer[offset + byteCount] = buffer[position + byteCount];
	}

	position += n;

	return n;
}
//---------------------------------------------------------------------------
void MemoryStream::Write(const uint8_t* _buffer, int offset, int count)
{
	if (!isOpen)
	{
		throw IOException();
	}

	const int i = position + count;
	if (i < 0)
	{
		throw IOException();
	}

	if (i > length)
	{
		if (i > capacity)
		{
			EnsureCapacity(i);
		}
		length = i;
	}

	while (--count >= 0)
	{
		buffer[position + count] = _buffer[offset + count];
	}

	position = i;
}
//---------------------------------------------------------------------------
std::vector<uint8_t> MemoryStream::ToArray() const
{
	return std::vector<uint8_t>(std::begin(buffer), std::begin(buffer) + length);
}
//---------------------------------------------------------------------------
bool MemoryStream::EnsureCapacity(int value)
{
	if (value < 0)
	{
		throw IOException();
	}
	if (value > capacity)
	{
		int newCapacity = value;
		if (newCapacity < 256)
		{
			newCapacity = 256;
		}

		if (newCapacity < capacity * 2)
		{
			newCapacity = capacity * 2;
		}

		if (static_cast<unsigned int>(capacity * 2) > MaxByteArrayLength)
		{
			newCapacity = static_cast<unsigned int>(value) > MaxByteArrayLength ? value : MaxByteArrayLength;
		}

		SetCapacity(newCapacity);

		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
