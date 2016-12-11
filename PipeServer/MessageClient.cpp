#include "MessageClient.hpp"

MessageClient::MessageClient(PipeStream& _pipe)
	: pipe(_pipe)
{

}
//---------------------------------------------------------------------------
void MessageClient::RegisterMessage(int type, const std::function<std::unique_ptr<IMessage>()>& creator)
{
	registeredMessages[type] = creator;
}
//---------------------------------------------------------------------------
std::unique_ptr<IMessage> MessageClient::Receive()
{
	MemoryStream ms;
	std::vector<uint8_t> buffer(256);
	do
	{
		auto length = pipe.Read(buffer.data(), 0, (int)buffer.size());
		ms.Write(buffer.data(), 0, length);
	} while (!pipe.IsMessageComplete());

	ms.SetPosition(0);

	BinaryReader br(ms);
	auto type = br.ReadInt32();

	auto it = registeredMessages.find(type);
	if (it != std::end(registeredMessages))
	{
		auto message = it->second();
		message->ReadFrom(br);

		return message;
	}

	return nullptr;
}
//---------------------------------------------------------------------------
void MessageClient::Send(const IMessage& message)
{
	MemoryStream ms;
	BinaryWriter bw(ms);

	bw.Write(message.GetMessageType());
	message.WriteTo(bw);

	auto data = ms.ToArray();
	pipe.Write(data.data(), 0, (int)data.size());
}
//---------------------------------------------------------------------------
