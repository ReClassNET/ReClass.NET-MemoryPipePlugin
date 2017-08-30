#pragma once

#include "PipeStream.hpp"

class NamedPipeServerStream : public PipeStream
{
public:
	static const int MaxAllowedServerInstances = -1;

	NamedPipeServerStream(const std::wstring& pipeName, PipeDirection direction, int maxNumberOfServerInstances, PipeTransmissionMode transmissionMode);

	NamedPipeServerStream(const std::wstring& pipeName, PipeDirection direction, int maxNumberOfServerInstances, PipeTransmissionMode transmissionMode, int inBufferSize, int outBufferSize);

	virtual ~NamedPipeServerStream() override;

private:
	void Create(const std::wstring& fullPipeName, PipeDirection direction, int maxNumberOfServerInstances, PipeTransmissionMode transmissionMode, int inBufferSize, int outBufferSize);

public:
	void WaitForConnection();

	void Disconnect();

private:
	void CheckConnectOperationsServer() const;

	void CheckDisconnectOperations() const;
};
