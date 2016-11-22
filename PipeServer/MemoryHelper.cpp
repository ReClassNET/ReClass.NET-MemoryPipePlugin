#include <windows.h>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <functional>

bool IsValidMemoryRange(LPCVOID address, int length)
{
	auto endAddress = static_cast<const uint8_t*>(address) + length;

	do
	{
		MEMORY_BASIC_INFORMATION info;
		if (!VirtualQuery(address, &info, sizeof(MEMORY_BASIC_INFORMATION)))
		{
			return false;
		}

		if (info.State != MEM_COMMIT)
		{
			return false;
		}

		switch (info.Protect)
		{
		case PAGE_EXECUTE_READ:
		case PAGE_EXECUTE_READWRITE:
		case PAGE_EXECUTE_WRITECOPY:
		case PAGE_READONLY:
		case PAGE_READWRITE:
		case PAGE_WRITECOPY:
			break;
		default:
			return false;
		}

		address = (uint8_t*)info.BaseAddress + info.RegionSize;
	} while (endAddress > address);

	return true;
}
//---------------------------------------------------------------------------
bool ReadMemory(LPCVOID address, std::vector<uint8_t>& buffer)
{
	if (!IsValidMemoryRange(address, (int)buffer.size()))
	{
		return false;
	}

	std::memcpy(buffer.data(), address, buffer.size());

	return true;
}
//---------------------------------------------------------------------------
bool WriteMemory(LPVOID address, const std::vector<uint8_t>& buffer)
{
	if (!IsValidMemoryRange(address, (int)buffer.size()))
	{
		return false;
	}

	DWORD oldProtect;
	if (VirtualProtect(address, buffer.size(), PAGE_EXECUTE_READWRITE, &oldProtect))
	{
		std::memcpy(address, buffer.data(), buffer.size());

		VirtualProtect(address, buffer.size(), oldProtect, nullptr);

		return true;
	}

	return false;
}
//---------------------------------------------------------------------------
void EnumerateRemoteSectionsAndModules(const std::function<void(const void*, const void*, std::wstring&&)>& moduleCallback, const std::function<void(const void*, const void*, std::wstring&&, int, int, int, std::wstring&&)>& sectionCallback)
{
	struct SectionInfo
	{
		LPVOID BaseAddress;
		SIZE_T RegionSize;
		WCHAR Name[IMAGE_SIZEOF_SHORT_NAME + 1];
		DWORD State;
		DWORD Protection;
		DWORD Type;
		WCHAR ModulePath[260];
	};
	std::vector<SectionInfo> sections;

	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);

	// First enumerate all memory sections.
	MEMORY_BASIC_INFORMATION memInfo;
	size_t address = (size_t)sysInfo.lpMinimumApplicationAddress;
	while (address < (size_t)sysInfo.lpMaximumApplicationAddress)
	{
		if (VirtualQuery((LPCVOID)address, &memInfo, sizeof(MEMORY_BASIC_INFORMATION)) != 0)
		{
			if (memInfo.State == MEM_COMMIT /*&& memInfo.Type == MEM_PRIVATE*/)
			{
				SectionInfo section = {};
				section.BaseAddress = memInfo.BaseAddress;
				section.RegionSize = memInfo.RegionSize;
				section.State = memInfo.State;
				section.Protection = memInfo.Protect;
				section.Type = memInfo.Type;

				sections.push_back(std::move(section));
			}
			address = (ULONG_PTR)memInfo.BaseAddress + memInfo.RegionSize;
		}
		else
		{
			address += 1024;
		}
	}

	struct UNICODE_STRING
	{
		USHORT Length;
		USHORT MaximumLength;
		PWSTR  Buffer;
	};

	struct LDR_MODULE
	{
		LIST_ENTRY InLoadOrderModuleList;
		LIST_ENTRY InMemoryOrderModuleList;
		LIST_ENTRY InInitializationOrderModuleList;
		PVOID BaseAddress;
		PVOID EntryPoint;
		ULONG SizeOfImage;
		UNICODE_STRING FullDllName;
		UNICODE_STRING BaseDllName;
		ULONG Flags;
		SHORT LoadCount;
		SHORT TlsIndex;
		LIST_ENTRY HashTableEntry;
		ULONG TimeDateStamp;
	};

	struct PEB_LDR_DATA
	{
		ULONG Length;
		BOOLEAN Initialized;
		PVOID SsHandle;
		LIST_ENTRY InLoadOrderModuleList;
		LIST_ENTRY InMemoryOrderModuleList;
		LIST_ENTRY InInitializationOrderModuleList;
	};

	struct PEB
	{
		BOOLEAN InheritedAddressSpace;
		BOOLEAN ReadImageFileExecOptions;
		BOOLEAN BeingDebugged;
		BOOLEAN Spare;
		HANDLE Mutant;
		PVOID ImageBaseAddress;
		PEB_LDR_DATA *LoaderData;
	};

	// Second enumerate all modules.
#ifdef _WIN64
	auto peb = reinterpret_cast<PEB*>(__readgsqword(0x60));
#else
	auto peb = reinterpret_cast<PEB*>(__readfsdword(0x30));
#endif
	auto ldr = reinterpret_cast<LDR_MODULE*>(peb->LoaderData->InLoadOrderModuleList.Flink);
	while (ldr->BaseAddress != nullptr)
	{
		moduleCallback(ldr->BaseAddress, (const void*)ldr->SizeOfImage, ldr->FullDllName.Buffer);

		auto it = std::lower_bound(std::begin(sections), std::end(sections), ldr->BaseAddress, [&sections](const SectionInfo& lhs, const LPVOID& rhs)
		{
			return lhs.BaseAddress < rhs;
		});

		auto dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(ldr->BaseAddress);
		auto ntHeader = reinterpret_cast<IMAGE_NT_HEADERS*>((intptr_t)ldr->BaseAddress + dosHeader->e_lfanew);

		int i = 0;
		for (auto sectionHeader = IMAGE_FIRST_SECTION(ntHeader); i < ntHeader->FileHeader.NumberOfSections; i++, sectionHeader++)
		{
			auto sectionAddress = (intptr_t)ldr->BaseAddress + sectionHeader->VirtualAddress;
			for (auto j = it; j != std::end(sections); ++j)
			{
				if (sectionAddress >= (intptr_t)j->BaseAddress && sectionAddress < (intptr_t)j->BaseAddress + (intptr_t)j->RegionSize)
				{
					// Copy the name because it is not null padded.
					char buffer[IMAGE_SIZEOF_SHORT_NAME + 1] = { 0 };
					std::memcpy(buffer, sectionHeader->Name, IMAGE_SIZEOF_SHORT_NAME);

					size_t convertedChars = 0;
					mbstowcs_s(&convertedChars, j->Name, IMAGE_SIZEOF_SHORT_NAME, buffer, _TRUNCATE);
					std::memcpy(j->ModulePath, ldr->FullDllName.Buffer, sizeof(SectionInfo::ModulePath));
					break;
				}
			}
		}

		ldr = reinterpret_cast<LDR_MODULE*>(ldr->InLoadOrderModuleList.Flink);
	}

	for (auto&& section : sections)
	{
		sectionCallback(section.BaseAddress, (const void*)section.RegionSize, section.Name, section.State, section.Protection, section.Type, section.ModulePath);
	}
}
//---------------------------------------------------------------------------
