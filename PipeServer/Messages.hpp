#pragma once

#include <string>

#include "ReClassNET_Plugin.hpp"

#include "PipeStream/BinaryReader.hpp"
#include "PipeStream/BinaryWriter.hpp"

class MessageClient;

enum class MessageType
{
	StatusResponse = 1,
	OpenProcessRequest = 2,
	CloseProcessRequest = 3,
	IsValidRequest = 4,
	ReadMemoryRequest = 5,
	ReadMemoryResponse = 6,
	WriteMemoryRequest = 7,
	EnumerateRemoteSectionsAndModulesRequest = 8,
	EnumerateRemoteSectionResponse = 9,
	EnumerateRemoteModuleResponse = 10,
	EnumerateProcessHandlesRequest = 11,
	EnumerateProcessHandlesResponse = 12,
	ClosePipeRequest = 13
};
//---------------------------------------------------------------------------
class IMessage
{
public:
	virtual ~IMessage() = default;

	virtual MessageType GetMessageType() const = 0;

	virtual void ReadFrom(BinaryReader& br) = 0;
	virtual void WriteTo(BinaryWriter& bw) const = 0;

	virtual bool Handle(MessageClient& client)
	{
		return true;
	}
};
//---------------------------------------------------------------------------
class StatusResponse : public IMessage
{
public:
	MessageType GetMessageType() const override { return MessageType::StatusResponse; }

	bool GetSuccess() const { return success; }

	StatusResponse()
		: success(false)
	{

	}

	StatusResponse(bool _success)
		: success(_success)
	{

	}

	void ReadFrom(BinaryReader& reader) override
	{
		success = reader.ReadBoolean();
	}

	void WriteTo(BinaryWriter& writer) const override
	{
		writer.Write(success);
	}

private:
	bool success;
};
//---------------------------------------------------------------------------
class OpenProcessRequest : public IMessage
{
public:
	MessageType GetMessageType() const override { return MessageType::OpenProcessRequest; }

	void ReadFrom(BinaryReader& reader) override
	{

	}

	void WriteTo(BinaryWriter& writer) const override
	{

	}

	bool Handle(MessageClient& client) override;
};
//---------------------------------------------------------------------------
class CloseProcessRequest : public IMessage
{
public:
	MessageType GetMessageType() const override { return MessageType::CloseProcessRequest; }

	void ReadFrom(BinaryReader& reader) override
	{

	}

	void WriteTo(BinaryWriter& writer) const override
	{

	}

	bool Handle(MessageClient& client) override;
};
//---------------------------------------------------------------------------
class IsValidRequest : public IMessage
{
public:
	MessageType GetMessageType() const override { return MessageType::IsValidRequest; }

	void ReadFrom(BinaryReader& reader) override
	{

	}

	void WriteTo(BinaryWriter& writer) const override
	{

	}

	bool Handle(MessageClient& client) override;
};
//---------------------------------------------------------------------------
class ReadMemoryRequest : public IMessage
{
public:
	MessageType GetMessageType() const override { return MessageType::ReadMemoryRequest; }

	const void* GetAddress() const { return address; }
	int GetSize() const { return size; }

	ReadMemoryRequest()
		: address(nullptr),
		  size(0)
	{

	}

	ReadMemoryRequest(const void* _address, int _size)
		: address(_address),
		  size(_size)
	{

	}

	void ReadFrom(BinaryReader& reader) override
	{
		address = reader.ReadIntPtr();
		size = reader.ReadInt32();
	}

	void WriteTo(BinaryWriter& writer) const override
	{
		writer.Write(address);
		writer.Write(size);
	}

	bool Handle(MessageClient& client) override;

private:
	const void* address;
	int size;
};
//---------------------------------------------------------------------------
class ReadMemoryResponse : public IMessage
{
public:
	MessageType GetMessageType() const override { return MessageType::ReadMemoryResponse; }

	const std::vector<uint8_t>& GetData() const { return data; }

	ReadMemoryResponse()
	{

	}

	ReadMemoryResponse(std::vector<uint8_t>&& _data)
		: data(std::move(_data))
	{

	}

	void ReadFrom(BinaryReader& reader) override
	{
		const auto size = reader.ReadInt32();
		data = reader.ReadBytes(size);
	}

	void WriteTo(BinaryWriter& writer) const override
	{
		writer.Write(static_cast<int>(data.size()));
		writer.Write(data.data(), 0, static_cast<int>(data.size()));
	}

private:
	std::vector<uint8_t> data;
};
//---------------------------------------------------------------------------
class WriteMemoryRequest : public IMessage
{
public:
	MessageType GetMessageType() const override { return MessageType::WriteMemoryRequest; }

	const void* GetAddress() const { return address; }
	const std::vector<uint8_t>& GetData() const { return data; }

	WriteMemoryRequest()
		: address(nullptr)
	{

	}

	WriteMemoryRequest(const void* _address, std::vector<uint8_t>&& _data)
		: address(_address),
		  data(std::move(_data))
	{

	}

	void ReadFrom(BinaryReader& reader) override
	{
		address = reader.ReadIntPtr();
		const auto size = reader.ReadInt32();
		data = reader.ReadBytes(size);
	}

	void WriteTo(BinaryWriter& writer) const override
	{
		writer.Write(address);
		writer.Write(static_cast<int>(data.size()));
		writer.Write(data.data(), 0, static_cast<int>(data.size()));
	}

	bool Handle(MessageClient& client) override;

private:
	const void* address;
	std::vector<uint8_t> data;
};
//---------------------------------------------------------------------------
class EnumerateRemoteSectionsAndModulesRequest : public IMessage
{
public:
	MessageType GetMessageType() const override { return MessageType::EnumerateRemoteSectionsAndModulesRequest; }

	void ReadFrom(BinaryReader& reader) override
	{

	}

	void WriteTo(BinaryWriter& writer) const override
	{

	}

	bool Handle(MessageClient& client) override;
};
//---------------------------------------------------------------------------
class EnumerateRemoteSectionResponse : public IMessage
{
public:
	MessageType GetMessageType() const override { return MessageType::EnumerateRemoteSectionResponse; }

	RC_Pointer GetBaseAddress() const { return baseAddress; }
	RC_Pointer GetRegionSize() const { return size; }
	SectionType GetType() const { return type; }
	SectionCategory GetCategory() const { return category; }
	SectionProtection GetProtection() const { return protection; }
	const std::wstring& GetName() const { return name; }
	const std::wstring& GetModulePath() const { return modulePath; }

	EnumerateRemoteSectionResponse()
		: baseAddress(nullptr),
		  size(nullptr),
		  type(SectionType::Unknown),
		  category(SectionCategory::Unknown),
		  protection(SectionProtection::NoAccess)
	{

	}

	EnumerateRemoteSectionResponse(RC_Pointer _baseAddress, RC_Pointer _size, SectionType _type, SectionCategory _category, SectionProtection _protection, std::wstring&& _name, std::wstring&& _modulePath)
		: baseAddress(_baseAddress),
		  size(_size),
		  type(_type),
		  category(_category),
		  protection(_protection),
		  name(std::move(_name)),
		  modulePath(std::move(_modulePath))
	{

	}

	void ReadFrom(BinaryReader& reader) override
	{
		baseAddress = reader.ReadIntPtr();
		size = reader.ReadIntPtr();
		type = static_cast<SectionType>(reader.ReadInt32());
		category = static_cast<SectionCategory>(reader.ReadInt32());
		protection = static_cast<SectionProtection>(reader.ReadInt32());
		name = reader.ReadString();
		modulePath = reader.ReadString();
	}

	void WriteTo(BinaryWriter& writer) const override
	{
		writer.Write(baseAddress);
		writer.Write(size);
		writer.Write(static_cast<int>(type));
		writer.Write(static_cast<int>(category));
		writer.Write(static_cast<int>(protection));
		writer.Write(name);
		writer.Write(modulePath);
	}

private:
	RC_Pointer baseAddress;
	RC_Pointer size;
	SectionType type;
	SectionCategory category;
	SectionProtection protection;
	std::wstring name;
	std::wstring modulePath;
};
//---------------------------------------------------------------------------
class EnumerateRemoteModuleResponse : public IMessage
{
public:
	MessageType GetMessageType() const override { return MessageType::EnumerateRemoteModuleResponse; }

	const void* GetBaseAddress() const { return baseAddress; }
	const void* GetRegionSize() const { return size; }
	const std::wstring& GetModulePath() const { return modulePath; }

	EnumerateRemoteModuleResponse()
		: baseAddress(nullptr),
		  size(nullptr)
	{

	}

	EnumerateRemoteModuleResponse(const void* _baseAddress, const void* _regionSize, std::wstring&& _modulePath)
		: baseAddress(_baseAddress),
		  size(_regionSize),
		  modulePath(std::move(_modulePath))
	{

	}

	void ReadFrom(BinaryReader& reader) override
	{
		baseAddress = reader.ReadIntPtr();
		size = reader.ReadIntPtr();
		modulePath = reader.ReadString();
	}

	void WriteTo(BinaryWriter& writer) const override
	{
		writer.Write(baseAddress);
		writer.Write(size);
		writer.Write(modulePath);
	}

private:
	const void* baseAddress;
	const void* size;
	std::wstring modulePath;
};
//---------------------------------------------------------------------------
