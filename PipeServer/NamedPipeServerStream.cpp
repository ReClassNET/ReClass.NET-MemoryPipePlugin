#include <string>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "NamedPipeServerStream.hpp"
#include "Exceptions.hpp"

NamedPipeServerStream::NamedPipeServerStream(const std::wstring& pipeName, PipeDirection direction, int maxNumberOfServerInstances, PipeTransmissionMode transmissionMode)
	: NamedPipeServerStream(pipeName, direction, maxNumberOfServerInstances, transmissionMode, 0, 0)
{

}
//---------------------------------------------------------------------------
NamedPipeServerStream::NamedPipeServerStream(const std::wstring& pipeName, PipeDirection direction, int maxNumberOfServerInstances, PipeTransmissionMode transmissionMode, int inBufferSize, int outBufferSize)
	: PipeStream(direction, transmissionMode)
{
	auto normalizedPipePath = (fs::path("\\\\.\\pipe") / pipeName).wstring();

	Create(normalizedPipePath, direction, maxNumberOfServerInstances, transmissionMode, inBufferSize, outBufferSize);
}
//---------------------------------------------------------------------------
NamedPipeServerStream::~NamedPipeServerStream()
{
	state = PipeState::Closed;
}
//---------------------------------------------------------------------------
void NamedPipeServerStream::Create(const std::wstring& fullPipeName, PipeDirection direction, int maxNumberOfServerInstances, PipeTransmissionMode transmissionMode, int inBufferSize, int outBufferSize)
{
	int pipeModes = (int)transmissionMode << 2 | (int)transmissionMode << 1;

	if (maxNumberOfServerInstances == MaxAllowedServerInstances)
	{
		maxNumberOfServerInstances = 255;
	}

	handle = SafePipeHandle(CreateNamedPipeW(fullPipeName.c_str(), (int)direction, pipeModes, maxNumberOfServerInstances, outBufferSize, inBufferSize, 0, nullptr), true);

	if (handle.IsInvalid())
	{
		throw IOException(GetLastError());
	}
}
//---------------------------------------------------------------------------
void NamedPipeServerStream::WaitForConnection()
{
	CheckConnectOperationsServer();

	if (!ConnectNamedPipe(handle.GetHandle(), nullptr))
	{
		int errorCode = GetLastError();
		if (errorCode != ERROR_PIPE_CONNECTED)
		{
			throw IOException(errorCode);
		}

		if (errorCode == ERROR_PIPE_CONNECTED && state == PipeState::Connected)
		{
			throw InvalidOperationException();
		}
	}
	state = PipeState::Connected;
}
//---------------------------------------------------------------------------
void NamedPipeServerStream::Disconnect()
{
	CheckDisconnectOperations();

	if (!DisconnectNamedPipe(handle.GetHandle()))
	{
		throw IOException(GetLastError());
	}

	state = PipeState::Disconnected;
}
//---------------------------------------------------------------------------
void NamedPipeServerStream::CheckConnectOperationsServer()
{
	if (state == PipeState::Closed || state == PipeState::Broken)
	{
		throw InvalidOperationException();
	}
}
//---------------------------------------------------------------------------
void NamedPipeServerStream::CheckDisconnectOperations()
{
	if (state == PipeState::WaitingToConnect || state == PipeState::Disconnected || state == PipeState::Closed)
	{
		throw InvalidOperationException();
	}
}
//---------------------------------------------------------------------------
