#pragma once

#include <string>

#include "BinaryReader.hpp"
#include "BinaryWriter.hpp"

class MessageClient;

class IMessage
{
public:
	virtual int GetType() const = 0;

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
	static const int StaticType = 1;
	virtual int GetType() const override { return StaticType; }

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
	static const int StaticType = 2;
	virtual int GetType() const override { return StaticType; }

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
	static const int StaticType = 3;
	virtual int GetType() const override { return StaticType; }

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
	static const int StaticType = 4;
	virtual int GetType() const override { return StaticType; }

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
	static const int StaticType = 5;
	virtual int GetType() const override { return StaticType; }

	const void* GetAddress() const { return address; }
	const int GetSize() const { return size; }

	ReadMemoryMessage()
		: address(0),
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
	static const int StaticType = 6;
	virtual int GetType() const override { return StaticType; }

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
		auto size = reader.ReadInt32();
		data = reader.ReadBytes(size);
	}

	virtual void WriteTo(BinaryWriter& writer) const override
	{
		writer.Write((int)data.size());
		writer.Write(data.data(), 0, (int)data.size());
	}

private:
	std::vector<uint8_t> data;
};
//---------------------------------------------------------------------------
class WriteMemoryMessage : public IMessage
{
public:
	static const int StaticType = 7;
	virtual int GetType() const override { return StaticType; }

	const void* GetAddress() const { return address; }
	const std::vector<uint8_t>& GetData() const { return data; }

	WriteMemoryMessage()
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
		auto size = reader.ReadInt32();
		data = reader.ReadBytes(size);
	}

	virtual void WriteTo(BinaryWriter& writer) const override
	{
		writer.Write(address);
		writer.Write((int)data.size());
		writer.Write(data.data(), 0, (int)data.size());
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
	static const int StaticType = 8;
	virtual int GetType() const override { return StaticType; }

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
	static const int StaticType = 9;
	virtual int GetType() const override { return StaticType; }

	const void* GetBaseAddress() const { return baseAddress; }
	const void* GetRegionSize() const { return regionSize; }
	const std::wstring& GetName() const { return name; }
	int GetState() const { return state; }
	int GetProtection() const { return protection; }
	int GetSectionType() const { return sectionType; }
	const std::wstring& GetModulePath() const { return modulePath; }

	EnumerateRemoteSectionCallbackMessage()
		: baseAddress(0),
		  regionSize(0),
		  state(0),
		  protection(0),
		  sectionType(0)
	{

	}

	EnumerateRemoteSectionCallbackMessage(const void* _baseAddress, const void* _regionSize, std::wstring&& _name, int _state, int _protection, int _type, std::wstring&& _modulePath)
		: baseAddress(_baseAddress),
		  regionSize(_regionSize),
		  name(std::move(_name)),
		  state(_state),
		  protection(_protection),
		  sectionType(_type),
		  modulePath(std::move(_modulePath))
	{

	}

	virtual void ReadFrom(BinaryReader& reader) override
	{
		baseAddress = reader.ReadIntPtr();
		regionSize = reader.ReadIntPtr();
		name = reader.ReadString();
		state = reader.ReadInt32();
		protection = reader.ReadInt32();
		sectionType = reader.ReadInt32();
		modulePath = reader.ReadString();
	}

	virtual void WriteTo(BinaryWriter& writer) const override
	{
		writer.Write(baseAddress);
		writer.Write(regionSize);
		writer.Write(name);
		writer.Write(state);
		writer.Write(protection);
		writer.Write(sectionType);
		writer.Write(modulePath);
	}

private:
	const void* baseAddress;
	const void* regionSize;
	std::wstring name;
	int state;
	int protection;
	int sectionType;
	std::wstring modulePath;
};
//---------------------------------------------------------------------------
class EnumerateRemoteModuleCallbackMessage : public IMessage
{
public:
	static const int StaticType = 10;
	virtual int GetType() const override { return StaticType; }

	const void* GetBaseAddress() const { return baseAddress; }
	const void* GetRegionSize() const { return regionSize; }
	const std::wstring& GetModulePath() const { return modulePath; }

	EnumerateRemoteModuleCallbackMessage()
		: baseAddress(0),
		  regionSize(0)
	{

	}

	EnumerateRemoteModuleCallbackMessage(const void* _baseAddress, const void* _regionSize, std::wstring&& _modulePath)
		: baseAddress(_baseAddress),
		  regionSize(_regionSize),
		  modulePath(std::move(_modulePath))
	{

	}

	virtual void ReadFrom(BinaryReader& reader) override
	{
		baseAddress = reader.ReadIntPtr();
		regionSize = reader.ReadIntPtr();
		modulePath = reader.ReadString();
	}

	virtual void WriteTo(BinaryWriter& writer) const override
	{
		writer.Write(baseAddress);
		writer.Write(regionSize);
		writer.Write(modulePath);
	}

private:
	const void* baseAddress;
	const void* regionSize;
	std::wstring modulePath;
};
//---------------------------------------------------------------------------
