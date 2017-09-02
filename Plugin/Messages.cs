using System;
using System.Diagnostics.Contracts;
using System.IO;
using ReClassNET.Memory;
using ReClassNET.Util;

namespace MemoryPipePlugin
{
	internal enum MessageType
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
	}

	internal interface IMessage
	{
		MessageType MessageType { get; }

		void ReadFrom(BinaryReader reader);
		void WriteTo(BinaryWriter writer);
	}

	[ContractClassFor(typeof(IMessage))]
	internal class MessageContract : IMessage
	{
		public MessageType MessageType => throw new NotImplementedException();

		public void ReadFrom(BinaryReader reader)
		{
			Contract.Requires(reader != null);

			throw new NotImplementedException();
		}

		public void WriteTo(BinaryWriter writer)
		{
			Contract.Requires(writer != null);

			throw new NotImplementedException();
		}
	}

	internal class StatusResponse : IMessage
	{
		public MessageType MessageType => MessageType.StatusResponse;

		public bool Success { get; private set; }

		public StatusResponse()
		{

		}

		public StatusResponse(bool success)
		{
			Success = success;
		}

		public void ReadFrom(BinaryReader reader)
		{
			Success = reader.ReadBoolean();
		}

		public void WriteTo(BinaryWriter writer)
		{
			writer.Write(Success);
		}
	}

	internal class OpenProcessRequest : IMessage
	{
		public MessageType MessageType => MessageType.OpenProcessRequest;

		public void ReadFrom(BinaryReader reader)
		{

		}

		public void WriteTo(BinaryWriter writer)
		{

		}
	}

	internal class CloseProcessRequest : IMessage
	{
		public MessageType MessageType => MessageType.CloseProcessRequest;

		public void ReadFrom(BinaryReader reader)
		{

		}

		public void WriteTo(BinaryWriter writer)
		{

		}
	}

	internal class IsValidRequest : IMessage
	{
		public MessageType MessageType => MessageType.IsValidRequest;

		public void ReadFrom(BinaryReader reader)
		{

		}

		public void WriteTo(BinaryWriter writer)
		{

		}
	}

	internal class ReadMemoryRequest : IMessage
	{
		public MessageType MessageType => MessageType.ReadMemoryRequest;

		public IntPtr Address { get; private set; }
		public int Size { get; private set; }

		public ReadMemoryRequest()
		{

		}

		public ReadMemoryRequest(IntPtr address, int size)
		{
			Address = address;
			Size = size;
		}

		public void ReadFrom(BinaryReader reader)
		{
			Address = reader.ReadIntPtr();
			Size = reader.ReadInt32();
		}

		public void WriteTo(BinaryWriter writer)
		{
			writer.Write(Address);
			writer.Write(Size);
		}
	}

	internal class ReadMemoryResponse : IMessage
	{
		public MessageType MessageType => MessageType.ReadMemoryResponse;

		public byte[] Data { get; private set; }

		public ReadMemoryResponse()
		{

		}

		public ReadMemoryResponse(byte[] data)
		{
			Data = data;
		}

		public void ReadFrom(BinaryReader reader)
		{
			var size = reader.ReadInt32();
			Data = reader.ReadBytes(size);
		}

		public void WriteTo(BinaryWriter writer)
		{
			writer.Write(Data.Length);
			writer.Write(Data);
		}
	}

	internal class WriteMemoryRequest : IMessage
	{
		public MessageType MessageType => MessageType.WriteMemoryRequest;

		public IntPtr Address { get; private set; }
		public byte[] Data { get; private set; }

		public WriteMemoryRequest()
		{

		}

		public WriteMemoryRequest(IntPtr address, byte[] data)
		{
			Address = address;
			Data = data;
		}

		public void ReadFrom(BinaryReader reader)
		{
			Address = reader.ReadIntPtr();
			var size = reader.ReadInt32();
			Data = reader.ReadBytes(size);
		}

		public void WriteTo(BinaryWriter writer)
		{
			writer.Write(Address);
			writer.Write(Data.Length);
			writer.Write(Data);
		}
	}

	internal class EnumerateRemoteSectionsAndModulesRequest : IMessage
	{
		public MessageType MessageType => MessageType.EnumerateRemoteSectionsAndModulesRequest;

		public void ReadFrom(BinaryReader reader)
		{

		}

		public void WriteTo(BinaryWriter writer)
		{

		}
	}

	internal class EnumerateRemoteSectionResponse : IMessage
	{
		public MessageType MessageType => MessageType.EnumerateRemoteSectionResponse;

		public IntPtr BaseAddress { get; private set; }
		public IntPtr Size { get; private set; }
		public SectionType Type { get; private set; }
		public SectionCategory Category { get; private set; }
		public SectionProtection Protection { get; private set; }
		public string Name { get; private set; }
		public string ModulePath { get; private set; }

		public EnumerateRemoteSectionResponse()
		{

		}

		public EnumerateRemoteSectionResponse(IntPtr baseAddress, IntPtr regionSize, SectionType type, SectionCategory category, SectionProtection protection, string name, string modulePath)
		{
			BaseAddress = baseAddress;
			Size = regionSize;
			Type = type;
			Category = category;
			Protection = protection;
			Name = name;
			ModulePath = modulePath;
		}

		public void ReadFrom(BinaryReader reader)
		{
			BaseAddress = reader.ReadIntPtr();
			Size = reader.ReadIntPtr();
			Type = (SectionType)reader.ReadInt32();
			Category = (SectionCategory)reader.ReadInt32();
			Protection = (SectionProtection)reader.ReadInt32();
			Name = reader.ReadString();
			ModulePath = reader.ReadString();
		}

		public void WriteTo(BinaryWriter writer)
		{
			writer.Write(BaseAddress);
			writer.Write(Size);
			writer.Write((int)Type);
			writer.Write((int)Category);
			writer.Write((int)Protection);
			writer.Write(Name);
			writer.Write(ModulePath);
		}
	}

	internal class EnumerateRemoteModuleResponse : IMessage
	{
		public MessageType MessageType => MessageType.EnumerateRemoteModuleResponse;

		public IntPtr BaseAddress { get; private set; }
		public IntPtr Size { get; private set; }
		public string Path { get; private set; }

		public EnumerateRemoteModuleResponse()
		{

		}

		public EnumerateRemoteModuleResponse(IntPtr baseAddress, IntPtr regionSize, string path)
		{
			BaseAddress = baseAddress;
			Size = regionSize;
			Path = path;
		}

		public void ReadFrom(BinaryReader reader)
		{
			BaseAddress = reader.ReadIntPtr();
			Size = reader.ReadIntPtr();
			Path = reader.ReadString();
		}

		public void WriteTo(BinaryWriter writer)
		{
			writer.Write(BaseAddress);
			writer.Write(Size);
			writer.Write(Path);
		}
	}
}
