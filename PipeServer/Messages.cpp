#include "Messages.hpp"
#include "MessageClient.hpp"

extern bool ReadMemory(LPCVOID, std::vector<uint8_t>&);
extern bool WriteMemory(LPVOID, const std::vector<uint8_t>&);
extern void EnumerateRemoteSectionsAndModules(const std::function<void(RC_Pointer, RC_Pointer, std::wstring&&)>&, const std::function<void(RC_Pointer, RC_Pointer, SectionType, SectionCategory, SectionProtection, std::wstring&&, std::wstring&&)>&);

bool OpenProcessMessage::Handle(MessageClient& client)
{
	client.Send(StatusMessage(true));

	return true;
}
//---------------------------------------------------------------------------
bool CloseProcessMessage::Handle(MessageClient& client)
{
	client.Send(StatusMessage(true));

	return false;
}
//---------------------------------------------------------------------------
bool IsValidMessage::Handle(MessageClient& client)
{
	client.Send(StatusMessage(true));

	return true;
}
//---------------------------------------------------------------------------
bool ReadMemoryMessage::Handle(MessageClient& client)
{
	std::vector<uint8_t> buffer(GetSize());
	buffer.resize(GetSize());

	if (ReadMemory(GetAddress(), buffer))
	{
		client.Send(ReadMemoryDataMessage(std::move(buffer)));
	}
	else
	{
		client.Send(StatusMessage(false));
	}

	return true;
}
//---------------------------------------------------------------------------
bool WriteMemoryMessage::Handle(MessageClient& client)
{
	const auto success = WriteMemory(const_cast<void*>(GetAddress()), GetData());

	client.Send(StatusMessage(success));

	return true;
}
//---------------------------------------------------------------------------
bool EnumerateRemoteSectionsAndModulesMessage::Handle(MessageClient& client)
{
	EnumerateRemoteSectionsAndModules(
		[&](auto p1, auto p2, auto p3) { client.Send(EnumerateRemoteModuleCallbackMessage(p1, p2, std::move(p3))); },
		[&](auto p1, auto p2, auto p3, auto p4, auto p5, auto p6, auto p7) { client.Send(EnumerateRemoteSectionCallbackMessage(p1, p2, p3, p4, p5, std::move(p6), std::move(p7))); }
	);
	
	// Report enumeration complete to client.
	client.Send(StatusMessage(true));

	return true;
}
//---------------------------------------------------------------------------
