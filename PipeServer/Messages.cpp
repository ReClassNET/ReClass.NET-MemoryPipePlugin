#include "Messages.hpp"
#include "MessageClient.hpp"

extern bool ReadMemory(LPCVOID, std::vector<uint8_t>&);
extern bool WriteMemory(LPVOID, const std::vector<uint8_t>&);
extern void EnumerateRemoteSectionsAndModules(const std::function<void(RC_Pointer, RC_Pointer, std::wstring&&)>&, const std::function<void(RC_Pointer, RC_Pointer, SectionType, SectionCategory, SectionProtection, std::wstring&&, std::wstring&&)>&);

bool OpenProcessRequest::Handle(MessageClient& client)
{
	client.Send(StatusResponse(true));

	return true;
}
//---------------------------------------------------------------------------
bool CloseProcessRequest::Handle(MessageClient& client)
{
	client.Send(StatusResponse(true));

	return false;
}
//---------------------------------------------------------------------------
bool IsValidRequest::Handle(MessageClient& client)
{
	client.Send(StatusResponse(true));

	return true;
}
//---------------------------------------------------------------------------
bool ReadMemoryRequest::Handle(MessageClient& client)
{
	std::vector<uint8_t> buffer(GetSize());
	buffer.resize(GetSize());

	if (ReadMemory(GetAddress(), buffer))
	{
		client.Send(ReadMemoryResponse(std::move(buffer)));
	}
	else
	{
		client.Send(StatusResponse(false));
	}

	return true;
}
//---------------------------------------------------------------------------
bool WriteMemoryRequest::Handle(MessageClient& client)
{
	const auto success = WriteMemory(const_cast<void*>(GetAddress()), GetData());

	client.Send(StatusResponse(success));

	return true;
}
//---------------------------------------------------------------------------
bool EnumerateRemoteSectionsAndModulesRequest::Handle(MessageClient& client)
{
	EnumerateRemoteSectionsAndModules(
		[&](auto p1, auto p2, auto p3) { client.Send(EnumerateRemoteModuleResponse(p1, p2, std::move(p3))); },
		[&](auto p1, auto p2, auto p3, auto p4, auto p5, auto p6, auto p7) { client.Send(EnumerateRemoteSectionResponse(p1, p2, p3, p4, p5, std::move(p6), std::move(p7))); }
	);
	
	// Report enumeration complete to client.
	client.Send(StatusResponse(true));

	return true;
}
//---------------------------------------------------------------------------
