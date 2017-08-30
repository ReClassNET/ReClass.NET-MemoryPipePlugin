#include "PipeStream.hpp"
#include "Exceptions.hpp"

PipeStream::PipeStream(PipeDirection direction)
	: PipeStream(direction, PipeTransmissionMode::Byte)
{

}
//---------------------------------------------------------------------------
PipeStream::PipeStream(PipeDirection direction, PipeTransmissionMode transmissionMode)
	: state(PipeState::WaitingToConnect),
	  isMessageComplete(true),
	  readMode(transmissionMode),
	  pipeDirection(direction)
{
	if (static_cast<int>(pipeDirection & PipeDirection::In) != 0)
	{
		canRead = true;
	}
	if (static_cast<int>(pipeDirection & PipeDirection::Out) != 0)
	{
		canWrite = true;
	}
}
//---------------------------------------------------------------------------
bool PipeStream::IsMessageComplete() const
{
	if (state == PipeState::WaitingToConnect || state == PipeState::Disconnected || state == PipeState::Closed || readMode != PipeTransmissionMode::Message)
	{
		throw InvalidOperationException();
	}

	return isMessageComplete;
}
//---------------------------------------------------------------------------
int PipeStream::Read(uint8_t* buffer, int offset, int count)
{
	if (!canRead)
	{
		throw InvalidOperationException();
	}

	CheckReadOperations();

	return ReadCore(buffer, offset, count);
}
//---------------------------------------------------------------------------
int PipeStream::ReadByte()
{
	CheckReadOperations();

	uint8_t buffer[1];
	const int n = ReadCore(buffer, 0, 1);

	if (n == 0)
	{
		return -1;
	}

	return static_cast<int>(buffer[0]);
}
//---------------------------------------------------------------------------
void PipeStream::Write(const uint8_t* buffer, int offset, int count)
{
	if (!canWrite)
	{
		throw InvalidOperationException();
	}

	CheckWriteOperations();

	WriteCore(buffer, offset, count);
}
//---------------------------------------------------------------------------
void PipeStream::WriteByte(uint8_t value)
{
	CheckWriteOperations();

	uint8_t buffer[1] = { value };
	WriteCore(buffer, 0, 1);
}
//---------------------------------------------------------------------------
void PipeStream::CheckReadOperations()
{
	if (state == PipeState::WaitingToConnect || state == PipeState::Disconnected || state == PipeState::Closed || handle.IsInvalid())
	{
		throw InvalidOperationException();
	}
}
//---------------------------------------------------------------------------
void PipeStream::CheckWriteOperations()
{
	if (state == PipeState::WaitingToConnect || state == PipeState::Disconnected || state == PipeState::Broken || state == PipeState::Closed || handle.IsInvalid())
	{
		throw InvalidOperationException();
	}
}
//---------------------------------------------------------------------------
void PipeStream::WinIOError(int errorCode)
{
	if (errorCode == ERROR_BROKEN_PIPE || errorCode == ERROR_PIPE_NOT_CONNECTED || errorCode == ERROR_NO_DATA)
	{
		state = PipeState::Broken;

		throw IOException(errorCode);

	}
	else if (errorCode == ERROR_HANDLE_EOF)
	{
		throw IOException();
	}
	else
	{
		if (errorCode == ERROR_INVALID_HANDLE)
		{
			handle = SafePipeHandle();
			state = PipeState::Broken;
		}

		throw IOException(errorCode);
	}
}
//---------------------------------------------------------------------------
int PipeStream::ReadCore(uint8_t* buffer, int offset, int count)
{
	int hr = 0;
	int r = ReadFileNative(handle, buffer, offset, count, hr);
	if (r == -1)
	{
		if (hr == ERROR_BROKEN_PIPE || hr == ERROR_PIPE_NOT_CONNECTED)
		{
			state = PipeState::Broken;
			r = 0;
		}
		else
		{
			throw IOException(hr);
		}
	}
	isMessageComplete = hr != ERROR_MORE_DATA;

	return r;
}
//---------------------------------------------------------------------------
int PipeStream::ReadFileNative(const SafePipeHandle& handle, uint8_t* buffer, int offset, int count, int& hr)
{
	if (count == 0)
	{
		hr = 0;
		return 0;
	}

	DWORD numBytesRead = 0;
	const int r = ReadFile(handle.GetHandle(), buffer + offset, count, &numBytesRead, nullptr);
	if (r == 0)
	{
		hr = GetLastError();

		if (hr == ERROR_MORE_DATA)
		{
			return numBytesRead;
		}

		return -1;
	}
	else
	{
		hr = 0;
	}

	return numBytesRead;
}
//---------------------------------------------------------------------------
void PipeStream::WriteCore(const uint8_t* buffer, int offset, int count) const
{
	int hr = 0;
	const int r = WriteFileNative(handle, buffer, offset, count, hr);

	if (r == -1)
	{
		throw IOException(hr);
	}
}
//---------------------------------------------------------------------------
int PipeStream::WriteFileNative(const SafePipeHandle& handle, const uint8_t* buffer, int offset, int count, int& hr)
{
	if (count == 0)
	{
		hr = 0;
		return 0;
	}

	DWORD numBytesWritten = 0;
	const int r = WriteFile(handle.GetHandle(), buffer + offset, count, &numBytesWritten, nullptr);
	if (r == 0)
	{
		hr = GetLastError();

		return -1;
	}
	else
	{
		hr = 0;
	}

	return numBytesWritten;
}
//---------------------------------------------------------------------------
