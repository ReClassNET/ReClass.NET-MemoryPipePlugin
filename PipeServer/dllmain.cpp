#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#include <process.h>

#include "NamedPipeServerStream.hpp"
#include "MessageClient.hpp"

std::wstring CreatePipeName()
{
	fs::path name(L"ReClass.NET");
#ifdef _WIN64
	name.append(L"x64");
#else
	name.append(L"x86");
#endif

	wchar_t szFileName[MAX_PATH];
	GetModuleFileNameW(nullptr, szFileName, MAX_PATH);
	name.append(fs::path(szFileName).filename());

	return name.wstring();
}
//---------------------------------------------------------------------------
MessageClient CreateClient(NamedPipeServerStream& pipe)
{
	MessageClient client(pipe);

	client.RegisterMessage(StatusMessage::StaticType, []() { return std::make_unique<StatusMessage>(); });
	client.RegisterMessage(OpenProcessMessage::StaticType, []() { return std::make_unique<OpenProcessMessage>(); });
	client.RegisterMessage(CloseProcessMessage::StaticType, []() { return std::make_unique<CloseProcessMessage>(); });
	client.RegisterMessage(IsValidMessage::StaticType, []() { return std::make_unique<IsValidMessage>(); });
	client.RegisterMessage(ReadMemoryMessage::StaticType, []() { return std::make_unique<ReadMemoryMessage>(); });
	client.RegisterMessage(ReadMemoryDataMessage::StaticType, []() { return std::make_unique<ReadMemoryDataMessage>(); });
	client.RegisterMessage(WriteMemoryMessage::StaticType, []() { return std::make_unique<WriteMemoryMessage>(); });
	client.RegisterMessage(EnumerateRemoteSectionsAndModulesMessage::StaticType, []() { return std::make_unique<EnumerateRemoteSectionsAndModulesMessage>(); });
	client.RegisterMessage(EnumerateRemoteSectionCallbackMessage::StaticType, []() { return std::make_unique<EnumerateRemoteSectionCallbackMessage>(); });
	client.RegisterMessage(EnumerateRemoteModuleCallbackMessage::StaticType, []() { return std::make_unique<EnumerateRemoteModuleCallbackMessage>(); });

	return client;
}
//---------------------------------------------------------------------------
void PipeThread(void*)
{
	auto name = CreatePipeName();
	
	while (true)
	{
		try
		{
			NamedPipeServerStream pipe(name, PipeDirection::InOut, 1, PipeTransmissionMode::Message);
			pipe.WaitForConnection();

			auto server = CreateClient(pipe);
			while (true)
			{
				auto message = server.Receive();
				if (message != nullptr)
				{
					if (!message->Handle(server))
					{
						break;
					}
				}
			}

			pipe.Disconnect();
		}
		catch (InvalidOperationException*)
		{

		}
		catch (IOException*)
		{

		}
		catch (...)
		{

		}
	}
}
//---------------------------------------------------------------------------
BOOL WINAPI DllMain(HMODULE handle, DWORD reason, PVOID reversed)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		_beginthread(PipeThread, 0, nullptr);

		return TRUE;
	}

	return FALSE;
}
//---------------------------------------------------------------------------
