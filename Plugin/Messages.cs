using System;
using System.Diagnostics.Contracts;
using System.IO;
using ReClassNET.Util;

namespace MemoryPipePlugin
{
	interface IMessage
	{
		int Type { get; }

		void ReadFrom(BinaryReader reader);
		void WriteTo(BinaryWriter writer);
	}

	[ContractClassFor(typeof(IMessage))]
	internal class ICodeGeneratorContract : IMessage
	{
		public int Type
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
		public int Type => StaticType;

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
		public int Type => StaticType;

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
		public int Type => StaticType;

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
		public int Type => StaticType;

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
		public int Type => StaticType;

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
		public int Type => StaticType;

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
		public int Type => StaticType;

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
		public int Type => StaticType;

		public void ReadFrom(BinaryReader reader)
		{

		}

		public void WriteTo(BinaryWriter writer)
		{

		}
	}

	class EnumerateRemoteSectionCallbackMessage : IMessage
	{
		public static int StaticType = 9;
		public int Type => StaticType;

		public IntPtr BaseAddress { get; private set; }
		public IntPtr RegionSize { get; private set; }
		public string Name { get; private set; }
		public NativeMethods.StateEnum State { get; private set; }
		public NativeMethods.AllocationProtectEnum Protection { get; private set; }
		public NativeMethods.TypeEnum SectionType { get; private set; }
		public string ModulePath { get; private set; }

		public EnumerateRemoteSectionCallbackMessage()
		{

		}

		public EnumerateRemoteSectionCallbackMessage(IntPtr baseAddress, IntPtr regionSize, string name, NativeMethods.StateEnum state, NativeMethods.AllocationProtectEnum protection, NativeMethods.TypeEnum type, string modulePath)
		{
			BaseAddress = baseAddress;
			RegionSize = regionSize;
			Name = name;
			State = state;
			Protection = protection;
			SectionType = type;
			ModulePath = modulePath;
		}

		public void ReadFrom(BinaryReader reader)
		{
			BaseAddress = reader.ReadIntPtr();
			RegionSize = reader.ReadIntPtr();
			Name = reader.ReadString();
			State = (NativeMethods.StateEnum)reader.ReadInt32();
			Protection = (NativeMethods.AllocationProtectEnum)reader.ReadInt32();
			SectionType = (NativeMethods.TypeEnum)reader.ReadInt32();
			ModulePath = reader.ReadString();
		}

		public void WriteTo(BinaryWriter writer)
		{
			writer.Write(BaseAddress);
			writer.Write(RegionSize);
			writer.Write(Name);
			writer.Write((int)State);
			writer.Write((int)Protection);
			writer.Write((int)SectionType);
			writer.Write(ModulePath);
		}
	}

	class EnumerateRemoteModuleCallbackMessage : IMessage
	{
		public static int StaticType = 10;
		public int Type => StaticType;

		public IntPtr BaseAddress { get; private set; }
		public IntPtr RegionSize { get; private set; }
		public string ModulePath { get; private set; }

		public EnumerateRemoteModuleCallbackMessage()
		{

		}

		public EnumerateRemoteModuleCallbackMessage(IntPtr baseAddress, IntPtr regionSize, string modulePath)
		{
			BaseAddress = baseAddress;
			RegionSize = regionSize;
			ModulePath = modulePath;
		}

		public void ReadFrom(BinaryReader reader)
		{
			BaseAddress = reader.ReadIntPtr();
			RegionSize = reader.ReadIntPtr();
			ModulePath = reader.ReadString();
		}

		public void WriteTo(BinaryWriter writer)
		{
			writer.Write(BaseAddress);
			writer.Write(RegionSize);
			writer.Write(ModulePath);
		}
	}
}
