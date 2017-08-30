#include <windows.h>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <functional>

#include "ReClassNET_Plugin.hpp"

bool IsValidMemoryRange(LPCVOID address, int length)
{
	const auto endAddress = static_cast<const uint8_t*>(address) + length;

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

		address = static_cast<uint8_t*>(info.BaseAddress) + info.RegionSize;
	} while (endAddress > address);

	return true;
}
//---------------------------------------------------------------------------
bool ReadMemory(LPCVOID address, std::vector<uint8_t>& buffer)
{
	if (!IsValidMemoryRange(address, static_cast<int>(buffer.size())))
	{
		return false;
	}

	std::memcpy(buffer.data(), address, buffer.size());

	return true;
}
//---------------------------------------------------------------------------
bool WriteMemory(LPVOID address, const std::vector<uint8_t>& buffer)
{
	if (!IsValidMemoryRange(address, static_cast<int>(buffer.size())))
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
void EnumerateRemoteSectionsAndModules(const std::function<void(RC_Pointer, RC_Pointer, std::wstring&&)>& moduleCallback, const std::function<void(RC_Pointer, RC_Pointer, SectionType, SectionCategory, SectionProtection, std::wstring&&, std::wstring&&)>& sectionCallback)
{
	std::vector<EnumerateRemoteSectionData> sections;

	// First enumerate all memory sections.
	MEMORY_BASIC_INFORMATION memInfo = { 0 };
	memInfo.RegionSize = 0x1000;
	size_t address = 0;
	while (VirtualQuery(reinterpret_cast<LPCVOID>(address), &memInfo, sizeof(MEMORY_BASIC_INFORMATION)) != 0 && address + memInfo.RegionSize > address)
	{
		if (memInfo.State == MEM_COMMIT)
		{
			EnumerateRemoteSectionData section = {};
			section.BaseAddress = memInfo.BaseAddress;
			section.Size = memInfo.RegionSize;

			switch (memInfo.Protect & 0xFF)
			{
			case PAGE_EXECUTE:
				section.Protection = SectionProtection::Execute;
				break;
			case PAGE_EXECUTE_READ:
				section.Protection = SectionProtection::Execute | SectionProtection::Read;
				break;
			case PAGE_EXECUTE_READWRITE:
			case PAGE_EXECUTE_WRITECOPY:
				section.Protection = SectionProtection::Execute | SectionProtection::Read | SectionProtection::Write;
				break;
			case PAGE_NOACCESS:
				section.Protection = SectionProtection::NoAccess;
				break;
			case PAGE_READONLY:
				section.Protection = SectionProtection::Read;
				break;
			case PAGE_READWRITE:
			case PAGE_WRITECOPY:
				section.Protection = SectionProtection::Read | SectionProtection::Write;
				break;
			}
			if ((memInfo.Protect & PAGE_GUARD) == PAGE_GUARD)
			{
				section.Protection |= SectionProtection::Guard;
			}

			switch (memInfo.Type)
			{
			case MEM_IMAGE:
				section.Type = SectionType::Image;
				break;
			case MEM_MAPPED:
				section.Type = SectionType::Mapped;
				break;
			case MEM_PRIVATE:
				section.Type = SectionType::Private;
				break;
			}

			section.Category = section.Type == SectionType::Private ? SectionCategory::HEAP : SectionCategory::Unknown;

			sections.push_back(std::move(section));
		}
		address = reinterpret_cast<size_t>(memInfo.BaseAddress) + memInfo.RegionSize;
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
	const auto peb = reinterpret_cast<PEB*>(__readgsqword(0x60));
#else
	const auto peb = reinterpret_cast<PEB*>(__readfsdword(0x30));
#endif
	auto ldr = reinterpret_cast<LDR_MODULE*>(peb->LoaderData->InLoadOrderModuleList.Flink);
	while (ldr->BaseAddress != nullptr)
	{
		moduleCallback(static_cast<RC_Pointer>(ldr->BaseAddress), reinterpret_cast<RC_Pointer>(static_cast<intptr_t>(ldr->SizeOfImage)), ldr->FullDllName.Buffer);

		const auto it = std::lower_bound(std::begin(sections), std::end(sections), ldr->BaseAddress, [&sections](const auto& lhs, const LPVOID& rhs)
		{
			return lhs.BaseAddress < rhs;
		});

		const auto dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(ldr->BaseAddress);
		const auto ntHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(reinterpret_cast<intptr_t>(ldr->BaseAddress) + dosHeader->e_lfanew);

		int i = 0;
		for (auto sectionHeader = IMAGE_FIRST_SECTION(ntHeader); i < ntHeader->FileHeader.NumberOfSections; i++, sectionHeader++)
		{
			const auto sectionAddress = reinterpret_cast<intptr_t>(ldr->BaseAddress) + static_cast<intptr_t>(sectionHeader->VirtualAddress);
			for (auto j = it; j != std::end(sections); ++j)
			{
				if (sectionAddress >= reinterpret_cast<intptr_t>(j->BaseAddress) && sectionAddress < reinterpret_cast<intptr_t>(j->BaseAddress) + static_cast<intptr_t>(j->Size))
				{
					// Copy the name because it is not null padded.
					char buffer[IMAGE_SIZEOF_SHORT_NAME + 1] = { 0 };
					std::memcpy(buffer, sectionHeader->Name, IMAGE_SIZEOF_SHORT_NAME);

					if (std::strcmp(buffer, ".text") == 0 || std::strcmp(buffer, "code") == 0)
					{
						j->Category = SectionCategory::CODE;
					}
					else if (std::strcmp(buffer, ".data") == 0 || std::strcmp(buffer, "data") == 0 || std::strcmp(buffer, ".rdata") == 0 || std::strcmp(buffer, ".idata") == 0)
					{
						j->Category = SectionCategory::DATA;
					}

					size_t convertedChars = 0;
					mbstowcs_s(&convertedChars, j->Name, IMAGE_SIZEOF_SHORT_NAME, buffer, _TRUNCATE);
					std::memcpy(j->ModulePath, ldr->FullDllName.Buffer, sizeof(EnumerateRemoteSectionData::ModulePath));
					break;
				}
			}
		}

		ldr = reinterpret_cast<LDR_MODULE*>(ldr->InLoadOrderModuleList.Flink);
	}

	for (auto&& section : sections)
	{
		sectionCallback(section.BaseAddress, reinterpret_cast<RC_Pointer>(section.Size), section.Type, section.Category, section.Protection, section.Name, section.ModulePath);
	}
}
//---------------------------------------------------------------------------
