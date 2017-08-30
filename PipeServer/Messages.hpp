#pragma once

#include <string>

#include "ReClassNET_Plugin.hpp"

#include "PipeStream/BinaryReader.hpp"
#include "PipeStream/BinaryWriter.hpp"

class MessageClient;

class IMessage
{
public:
	virtual ~IMessage() = default;

	virtual int GetMessageType() const = 0;

	virtual void ReadFrom(BinaryReader& br) = 0;
	virtual void WriteTo(BinaryWriter& bw) const = 0;

	virtual bool Handle(MessageClient& client)
	{
		return true;
	}
};
//---------------------------------------------------------------------------
class StatusMessage : public IMessage
{
public:
	static const int StaticMessageType = 1;
	virtual int GetMessageType() const override { return StaticMessageType; }

	bool GetSuccess() const { return success; }

	StatusMessage()
		: success(false)
	{

	}

	StatusMessage(bool _success)
		: success(_success)
	{

	}

	virtual void ReadFrom(BinaryReader& reader) override
	{
		success = reader.ReadBoolean();
	}

	virtual void WriteTo(BinaryWriter& writer) const override
	{
		writer.Write(success);
	}

private:
	bool success;
};
//---------------------------------------------------------------------------
class OpenProcessMessage : public IMessage
{
public:
	static const int StaticMessageType = 2;
	virtual int GetMessageType() const override { return StaticMessageType; }

	virtual void ReadFrom(BinaryReader& reader) override
	{

	}

	virtual void WriteTo(BinaryWriter& writer) const override
	{

	}

	virtual bool Handle(MessageClient& client) override;
};
//---------------------------------------------------------------------------
class CloseProcessMessage : public IMessage
{
public:
	static const int StaticMessageType = 3;
	virtual int GetMessageType() const override { return StaticMessageType; }

	virtual void ReadFrom(BinaryReader& reader) override
	{

	}

	virtual void WriteTo(BinaryWriter& writer) const override
	{

	}

	virtual bool Handle(MessageClient& client) override;
};
//---------------------------------------------------------------------------
class IsValidMessage : public IMessage
{
public:
	static const int StaticMessageType = 4;
	virtual int GetMessageType() const override { return StaticMessageType; }

	virtual void ReadFrom(BinaryReader& reader) override
	{

	}

	virtual void WriteTo(BinaryWriter& writer) const override
	{

	}

	virtual bool Handle(MessageClient& client) override;
};
//---------------------------------------------------------------------------
class ReadMemoryMessage : public IMessage
{
public:
	static const int StaticMessageType = 5;
	virtual int GetMessageType() const override { return StaticMessageType; }

	const void* GetAddress() const { return address; }
	int GetSize() const { return size; }

	ReadMemoryMessage()
		: address(nullptr),
		  size(0)
	{

	}

	ReadMemoryMessage(const void* _address, int _size)
		: address(_address),
		  size(_size)
	{

	}

	virtual void ReadFrom(BinaryReader& reader) override
	{
		address = reader.ReadIntPtr();
		size = reader.ReadInt32();
	}

	virtual void WriteTo(BinaryWriter& writer) const override
	{
		writer.Write(address);
		writer.Write(size);
	}

	virtual bool Handle(MessageClient& client) override;

private:
	const void* address;
	int size;
};
//---------------------------------------------------------------------------
class ReadMemoryDataMessage : public IMessage
{
public:
	static const int StaticMessageType = 6;
	virtual int GetMessageType() const override { return StaticMessageType; }

	const std::vector<uint8_t>& GetData() const { return data; }

	ReadMemoryDataMessage()
	{

	}

	ReadMemoryDataMessage(std::vector<uint8_t>&& _data)
		: data(std::move(_data))
	{

	}

	virtual void ReadFrom(BinaryReader& reader) override
	{
		const auto size = reader.ReadInt32();
		data = reader.ReadBytes(size);
	}

	virtual void WriteTo(BinaryWriter& writer) const override
	{
		writer.Write(static_cast<int>(data.size()));
		writer.Write(data.data(), 0, static_cast<int>(data.size()));
	}

private:
	std::vector<uint8_t> data;
};
//---------------------------------------------------------------------------
class WriteMemoryMessage : public IMessage
{
public:
	static const int StaticMessageType = 7;
	virtual int GetMessageType() const override { return StaticMessageType; }

	const void* GetAddress() const { return address; }
	const std::vector<uint8_t>& GetData() const { return data; }

	WriteMemoryMessage()
		: address(nullptr)
	{

	}

	WriteMemoryMessage(const void* _address, std::vector<uint8_t>&& _data)
		: address(_address),
		  data(std::move(_data))
	{

	}

	virtual void ReadFrom(BinaryReader& reader) override
	{
		address = reader.ReadIntPtr();
		const auto size = reader.ReadInt32();
		data = reader.ReadBytes(size);
	}

	virtual void WriteTo(BinaryWriter& writer) const override
	{
		writer.Write(address);
		writer.Write(static_cast<int>(data.size()));
		writer.Write(data.data(), 0, static_cast<int>(data.size()));
	}

	virtual bool Handle(MessageClient& client) override;

private:
	const void* address;
	std::vector<uint8_t> data;
};
//---------------------------------------------------------------------------
class EnumerateRemoteSectionsAndModulesMessage : public IMessage
{
public:
	static const int StaticMessageType = 8;
	virtual int GetMessageType() const override { return StaticMessageType; }

	virtual void ReadFrom(BinaryReader& reader) override
	{

	}

	virtual void WriteTo(BinaryWriter& writer) const override
	{

	}

	virtual bool Handle(MessageClient& client) override;
};
//---------------------------------------------------------------------------
class EnumerateRemoteSectionCallbackMessage : public IMessage
{
public:
	static const int StaticMessageType = 9;
	virtual int GetMessageType() const override { return StaticMessageType; }

	RC_Pointer GetBaseAddress() const { return baseAddress; }
	RC_Pointer GetRegionSize() const { return size; }
	SectionType GetType() const { return type; }
	SectionCategory GetCategory() const { return category; }
	SectionProtection GetProtection() const { return protection; }
	const std::wstring& GetName() const { return name; }
	const std::wstring& GetModulePath() const { return modulePath; }

	EnumerateRemoteSectionCallbackMessage()
		: baseAddress(nullptr),
		  size(nullptr),
		  type(SectionType::Unknown),
		  category(SectionCategory::Unknown),
		  protection(SectionProtection::NoAccess)
	{

	}

	EnumerateRemoteSectionCallbackMessage(RC_Pointer _baseAddress, RC_Pointer _size, SectionType _type, SectionCategory _category, SectionProtection _protection, std::wstring&& _name, std::wstring&& _modulePath)
		: baseAddress(_baseAddress),
		  size(_size),
		  type(_type),
		  category(_category),
		  protection(_protection),
		  name(std::move(_name)),
		  modulePath(std::move(_modulePath))
	{

	}

	virtual void ReadFrom(BinaryReader& reader) override
	{
		baseAddress = reader.ReadIntPtr();
		size = reader.ReadIntPtr();
		type = static_cast<SectionType>(reader.ReadInt32());
		category = static_cast<SectionCategory>(reader.ReadInt32());
		protection = static_cast<SectionProtection>(reader.ReadInt32());
		name = reader.ReadString();
		modulePath = reader.ReadString();
	}

	virtual void WriteTo(BinaryWriter& writer) const override
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
class EnumerateRemoteModuleCallbackMessage : public IMessage
{
public:
	static const int StaticMessageType = 10;
	virtual int GetMessageType() const override { return StaticMessageType; }

	const void* GetBaseAddress() const { return baseAddress; }
	const void* GetRegionSize() const { return size; }
	const std::wstring& GetModulePath() const { return modulePath; }

	EnumerateRemoteModuleCallbackMessage()
		: baseAddress(nullptr),
		  size(nullptr)
	{

	}

	EnumerateRemoteModuleCallbackMessage(const void* _baseAddress, const void* _regionSize, std::wstring&& _modulePath)
		: baseAddress(_baseAddress),
		  size(_regionSize),
		  modulePath(std::move(_modulePath))
	{

	}

	virtual void ReadFrom(BinaryReader& reader) override
	{
		baseAddress = reader.ReadIntPtr();
		size = reader.ReadIntPtr();
		modulePath = reader.ReadString();
	}

	virtual void WriteTo(BinaryWriter& writer) const override
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
