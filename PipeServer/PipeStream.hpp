#pragma once

#include "Stream.hpp"
#include "SafeHandle.hpp"

class SafePipeHandle : public SafeHandleZeroOrMinusOneIsInvalid
{
public:
	SafePipeHandle()
		: SafeHandleZeroOrMinusOneIsInvalid(true)
	{
	}

	SafePipeHandle(HANDLE preexistingHandle, bool ownsHandle)
		: SafeHandleZeroOrMinusOneIsInvalid(ownsHandle)
	{
		SetHandle(preexistingHandle);
	}

	SafePipeHandle(SafePipeHandle&& copy) noexcept
		: SafeHandleZeroOrMinusOneIsInvalid(true)
	{
		handle = copy.handle;

		copy.ownsHandle = false;
		copy.handle = nullptr;
	}

	virtual ~SafePipeHandle() override
	{
		if (handle != nullptr && ownsHandle)
		{
			CloseHandle(handle);
			
			handle = nullptr;
		}
	}

	SafePipeHandle& operator=(SafePipeHandle&& copy) noexcept
	{
		handle = copy.handle;

		copy.handle = nullptr;

		return *this;
	}
};

enum class PipeDirection
{
	In = 1,
	Out = 2,
	InOut = In | Out,
};

inline PipeDirection operator |(PipeDirection a, PipeDirection b)
{
	return static_cast<PipeDirection>(static_cast<int>(a) | static_cast<int>(b));
}

inline PipeDirection operator &(PipeDirection a, PipeDirection b)
{
	return static_cast<PipeDirection>(static_cast<int>(a) & static_cast<int>(b));
}

enum class PipeTransmissionMode
{
	Byte = 0,
	Message = 1,
};

enum class PipeOptions
{
	None = 0x0,
	WriteThrough = static_cast<int>(0x80000000),
	Asynchronous = static_cast<int>(0x40000000),
};

enum class PipeState
{
	WaitingToConnect = 0,
	Connected = 1,
	Broken = 2,
	Disconnected = 3,
	Closed = 4,
};

class PipeStream : public Stream
{
protected:
	PipeStream(PipeDirection direction);

	PipeStream(PipeDirection direction, PipeTransmissionMode transmissionMode);

public:
	bool IsMessageComplete() const;

	virtual int Read(uint8_t* buffer, int offset, int count) override;

	virtual int ReadByte() override;

	void Write(const uint8_t* buffer, int offset, int count) override;

	virtual void WriteByte(uint8_t value) override;

private:
	void CheckReadOperations();

	void CheckWriteOperations();

	void WinIOError(int errorCode);

	int ReadCore(uint8_t* buffer, int offset, int count);

	static int ReadFileNative(const SafePipeHandle& handle, uint8_t* buffer, int offset, int count, int& hr);

	void WriteCore(const uint8_t* buffer, int offset, int count) const;

	static int WriteFileNative(const SafePipeHandle& handle, const uint8_t* buffer, int offset, int count, int& hr);

protected:
	SafePipeHandle handle;
	PipeState state;

private:
	bool canRead;
	bool canWrite;
	bool isMessageComplete;
	PipeTransmissionMode readMode;
	PipeDirection pipeDirection;
};
