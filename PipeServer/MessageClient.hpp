#pragma once

#include <unordered_map>
#include <memory>
#include <functional>

#include "PipeStream/PipeStream.hpp"
#include "Messages.hpp"

class MessageClient
{
public:
	MessageClient(PipeStream& pipe);

	void RegisterMessage(int type, const std::function<std::unique_ptr<IMessage>()>& creator);

	std::unique_ptr<IMessage> Receive();

	void Send(const IMessage& message) const;

private:
	PipeStream& pipe;

	std::unordered_map<int, std::function<std::unique_ptr<IMessage>()>> registeredMessages;
};
