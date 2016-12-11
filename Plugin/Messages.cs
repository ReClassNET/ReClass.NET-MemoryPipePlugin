using System;
using System.Diagnostics.Contracts;
using System.IO;
using ReClassNET.Memory;
using ReClassNET.Util;

namespace MemoryPipePlugin
{
	interface IMessage
	{
		int MessageType { get; }

		void ReadFrom(BinaryReader reader);
		void WriteTo(BinaryWriter writer);
	}

	[ContractClassFor(typeof(IMessage))]
	internal class ICodeGeneratorContract : IMessage
	{
		public int MessageType
		{
			get
			{
				throw new NotImplementedException();
			}
		}

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

	class StatusMessage : IMessage
	{
		public static int StaticType = 1;
		public int MessageType => StaticType;

		public bool Success { get; private set; }

		public StatusMessage()
		{

		}

		public StatusMessage(bool success)
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

	class OpenProcessMessage : IMessage
	{
		public static int StaticType = 2;
		public int MessageType => StaticType;

		public void ReadFrom(BinaryReader reader)
		{

		}

		public void WriteTo(BinaryWriter writer)
		{

		}
	}

	class CloseProcessMessage : IMessage
	{
		public static int StaticType = 3;
		public int MessageType => StaticType;

		public void ReadFrom(BinaryReader reader)
		{

		}

		public void WriteTo(BinaryWriter writer)
		{

		}
	}

	class IsValidMessage : IMessage
	{
		public static int StaticType = 4;
		public int MessageType => StaticType;

		public void ReadFrom(BinaryReader reader)
		{

		}

		public void WriteTo(BinaryWriter writer)
		{

		}
	}

	class ReadMemoryMessage : IMessage
	{
		public static int StaticType = 5;
		public int MessageType => StaticType;

		public IntPtr Address { get; private set; }
		public int Size { get; private set; }

		public ReadMemoryMessage()
		{

		}

		public ReadMemoryMessage(IntPtr address, int size)
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

	class ReadMemoryDataMessage : IMessage
	{
		public static int StaticType = 6;
		public int MessageType => StaticType;

		public byte[] Data { get; private set; }

		public ReadMemoryDataMessage()
		{

		}

		public ReadMemoryDataMessage(byte[] data)
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

	class WriteMemoryMessage : IMessage
	{
		public static int StaticType = 7;
		public int MessageType => StaticType;

		public IntPtr Address { get; private set; }
		public byte[] Data { get; private set; }

		public WriteMemoryMessage()
		{

		}

		public WriteMemoryMessage(IntPtr address, byte[] data)
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

	class EnumerateRemoteSectionsAndModulesMessage : IMessage
	{
		public static int StaticType = 8;
		public int MessageType => StaticType;

		public void ReadFrom(BinaryReader reader)
		{

		}

		public void WriteTo(BinaryWriter writer)
		{

		}
	}

	class EnumerateRemoteSectionCallbackMessage : IMessage
	{
		public static int StaticMessageType = 9;
		public int MessageType => StaticMessageType;

		public IntPtr BaseAddress { get; private set; }
		public IntPtr Size { get; private set; }
		public SectionType Type { get; private set; }
		public SectionProtection Protection { get; private set; }
		public string Name { get; private set; }
		public string ModulePath { get; private set; }

		public EnumerateRemoteSectionCallbackMessage()
		{

		}

		public EnumerateRemoteSectionCallbackMessage(IntPtr baseAddress, IntPtr regionSize, SectionType type, SectionProtection protection, string name, string modulePath)
		{
			BaseAddress = baseAddress;
			Size = regionSize;
			Type = type;
			Protection = protection;
			Name = name;
			ModulePath = modulePath;
		}

		public void ReadFrom(BinaryReader reader)
		{
			BaseAddress = reader.ReadIntPtr();
			Size = reader.ReadIntPtr();
			Type = (SectionType)reader.ReadInt32();
			Protection = (SectionProtection)reader.ReadInt32();
			Name = reader.ReadString();
			ModulePath = reader.ReadString();
		}

		public void WriteTo(BinaryWriter writer)
		{
			writer.Write(BaseAddress);
			writer.Write(Size);
			writer.Write((int)Type);
			writer.Write((int)Protection);
			writer.Write(Name);
			writer.Write(ModulePath);
		}
	}

	class EnumerateRemoteModuleCallbackMessage : IMessage
	{
		public static int StaticType = 10;
		public int MessageType => StaticType;

		public IntPtr BaseAddress { get; private set; }
		public IntPtr Size { get; private set; }
		public string Path { get; private set; }

		public EnumerateRemoteModuleCallbackMessage()
		{

		}

		public EnumerateRemoteModuleCallbackMessage(IntPtr baseAddress, IntPtr regionSize, string path)
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
