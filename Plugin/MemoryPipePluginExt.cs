using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.Drawing;
using System.IO;
using System.IO.Pipes;
using System.Linq;
using System.Runtime.InteropServices;
using ReClassNET.Plugins;
using RGiesecke.DllExport;
using static ReClassNET.Memory.NativeHelper;

namespace MemoryPipePlugin
{
	public class MemoryPipePluginExt : Plugin
	{
		private const string PipePrefix = @"\\.\pipe\";

		private static object sync = new object();

		private static IPluginHost host;

		private static Dictionary<IntPtr, MessageClient> openPipes;

		public override Image Icon => Properties.Resources.logo;

		public override bool Initialize(IPluginHost host)
		{
			Contract.Requires(host != null);

			//System.Diagnostics.Debugger.Launch();

			if (MemoryPipePluginExt.host != null)
			{
				Terminate();
			}

			if (host == null)
			{
				throw new ArgumentNullException(nameof(host));
			}

			MemoryPipePluginExt.host = host;

			openPipes = new Dictionary<IntPtr, MessageClient>();

			return true;
		}

		public override void Terminate()
		{
			foreach (var kv in openPipes)
			{
				kv.Value.Pipe.Dispose();
			}
			openPipes.Clear();

			host = null;
		}

		/// <summary>Gets a <see cref="MessageClient"/> by its plugin internal identifier.</summary>
		/// <param name="id">The identifier.</param>
		/// <returns>The client or null if the identifier doesn't exist.</returns>
		private static MessageClient GetClientById(IntPtr id)
		{
			MessageClient client;
			openPipes.TryGetValue(id, out client);
			return client;
		}

		/// <summary>Logs the exception and removes client.</summary>
		/// <param name="id">The identifier.</param>
		/// <param name="ex">The exception.</param>
		private static void LogErrorAndRemoveClient(IntPtr id, Exception ex)
		{
			Contract.Requires(ex != null);

			GetClientById(id)?.Pipe?.Dispose();

			openPipes.Remove(id);

			host.Logger.Log(ex);
		}

		/// <summary>Enumerates all pipes created by the ReClass.NET PipeServer.</summary>
		/// <returns>An enumerator to all pipes.</returns>
		private static IEnumerable<string> GetPipes()
		{
			return Directory.GetFiles(PipePrefix).Where(p => p.Contains("ReClass.NET"));
		}

		/// <summary>Queries if the process is valid. If the pipe isn't broken we can assume the process is valid.</summary>
		/// <remarks><![CDATA[
		/// Protocoll:
		/// -> IsValidMessage
		/// <- StatusMessage
		/// ]]></remarks>
		/// <param name="process">The process to check.</param>
		/// <returns>True if the process is valid, false if not.</returns>
		[DllExport(CallingConvention = CallingConvention.StdCall)]
		public static bool IsProcessValid(IntPtr process)
		{
			lock (sync)
			{
				MessageClient client;
				if (openPipes.TryGetValue(process, out client))
				{
					try
					{
						client.Send(new IsValidMessage());
						var message = client.Receive() as StatusMessage;

						return message.Success;
					}
					catch (Exception ex)
					{
						LogErrorAndRemoveClient(process, ex);
					}
				}

				return false;
			}
		}

		/// <summary>Opens the pipe to the target process.</summary>
		/// <remarks><![CDATA[
		/// Protocoll:
		/// -> OpenProcessMessage
		/// <- StatusMessage
		/// ]]></remarks>
		/// <param name="pid">The process id.</param>
		/// <param name="desiredAccess">The desired access. (ignored)</param>
		/// <returns>A plugin internal handle to the process.</returns>
		[DllExport(CallingConvention = CallingConvention.StdCall)]
		private static IntPtr OpenRemoteProcess(int pid, int desiredAccess)
		{
			lock (sync)
			{
				try
				{
					var pipePath = GetPipes().Where(p => p.GetHashCode() == pid).FirstOrDefault();
					if (pipePath == null)
					{
						return IntPtr.Zero;
					}

					var pipeName = pipePath.Substring(PipePrefix.Length);

					var pipe = new NamedPipeClientStream(".", pipeName, PipeDirection.InOut);
					pipe.Connect();
					pipe.ReadMode = PipeTransmissionMode.Message;

					var client = new MessageClient(pipe);

					client.RegisteredMessages.Add(StatusMessage.StaticType, () => new StatusMessage());
					client.RegisteredMessages.Add(OpenProcessMessage.StaticType, () => new OpenProcessMessage());
					client.RegisteredMessages.Add(CloseProcessMessage.StaticType, () => new CloseProcessMessage());
					client.RegisteredMessages.Add(IsValidMessage.StaticType, () => new IsValidMessage());
					client.RegisteredMessages.Add(ReadMemoryMessage.StaticType, () => new ReadMemoryMessage());
					client.RegisteredMessages.Add(ReadMemoryDataMessage.StaticType, () => new ReadMemoryDataMessage());
					client.RegisteredMessages.Add(WriteMemoryMessage.StaticType, () => new WriteMemoryMessage());
					client.RegisteredMessages.Add(EnumerateRemoteSectionsAndModulesMessage.StaticType, () => new EnumerateRemoteSectionsAndModulesMessage());
					client.RegisteredMessages.Add(EnumerateRemoteSectionCallbackMessage.StaticMessageType, () => new EnumerateRemoteSectionCallbackMessage());
					client.RegisteredMessages.Add(EnumerateRemoteModuleCallbackMessage.StaticType, () => new EnumerateRemoteModuleCallbackMessage());

					var handle = pipe.SafePipeHandle.DangerousGetHandle();

					openPipes.Add(handle, client);

					try
					{
						client.Send(new OpenProcessMessage());
						client.Receive(); // swallow the StatusMessage

						return handle;
					}
					catch (Exception ex)
					{
						LogErrorAndRemoveClient(handle, ex);
					}
				}
				catch (Exception ex)
				{
					host.Logger.Log(ex);
				}
			}

			return IntPtr.Zero;
		}

		/// <summary>Closes the pipe to the remote process.</summary>
		/// <remarks><![CDATA[
		/// Protocoll:
		/// -> CloseProcessMessage
		/// <- StatusMessage
		/// ]]></remarks>
		/// <param name="process">The process to close.</param>
		[DllExport(CallingConvention = CallingConvention.StdCall)]
		private static void CloseRemoteProcess(IntPtr process)
		{
			lock (sync)
			{
				MessageClient client;
				if (openPipes.TryGetValue(process, out client))
				{
					openPipes.Remove(process);

					try
					{
						client.Send(new CloseProcessMessage());
						client.Receive(); // swallow the StatusMessage
					}
					catch
					{

					}

					client.Pipe.Dispose();
				}
			}
		}

		/// <summary>Reads memory of the remote process through the pipe.</summary>
		/// <remarks><![CDATA[
		/// Protocoll:
		/// -> ReadMemoryMessage
		/// <- StatusMessage, if an error occured
		/// <- ReadMemoryDataMessage, if no error occured
		/// ]]></remarks>
		/// <param name="process">The process to read from.</param>
		/// <param name="address">The address to read from.</param>
		/// <param name="buffer">The buffer to read into.</param>
		/// <param name="size">The size of the memory to read.</param>
		/// <returns>True if it succeeds, false if it fails.</returns>
		[DllExport(CallingConvention = CallingConvention.StdCall)]
		private static bool ReadRemoteMemory(IntPtr process, IntPtr address, IntPtr buffer, int size)
		{
			lock (sync)
			{
				var client = GetClientById(process);
				if (client != null)
				{
					try
					{
						client.Send(new ReadMemoryMessage(address, size));
						var response = client.Receive();
						var statusMessage = response as StatusMessage;
						if (statusMessage != null)
						{
							return statusMessage.Success;
						}
						var memoryResponse = response as ReadMemoryDataMessage;
						if (memoryResponse != null)
						{
							if (memoryResponse.Data.Length == size)
							{
								Marshal.Copy(memoryResponse.Data, 0, buffer, size);

								return true;
							}
						}
					}
					catch (Exception ex)
					{
						LogErrorAndRemoveClient(process, ex);
					}
				}

				return false;
			}
		}

		/// <summary>Writes memory to the remote process.</summary>
		/// <remarks><![CDATA[
		/// Protocoll:
		/// -> WriteMemoryMessage
		/// <- StatusMessage
		/// ]]></remarks>
		/// <param name="process">The process to write to.</param>
		/// <param name="address">The address to write to.</param>
		/// <param name="buffer">The memory to write.</param>
		/// <param name="size">The size of the memory to write.</param>
		/// <returns>True if it succeeds, false if it fails.</returns>
		[DllExport(CallingConvention = CallingConvention.StdCall)]
		private static bool WriteRemoteMemory(IntPtr process, IntPtr address, IntPtr buffer, int size)
		{
			lock (sync)
			{
				var client = GetClientById(process);
				if (client != null)
				{
					try
					{
						var data = new byte[size];
						Marshal.Copy(buffer, data, 0, size);

						client.Send(new WriteMemoryMessage(address, data));
						var message = client.Receive() as StatusMessage;
						return message.Success;
					}
					catch (Exception ex)
					{
						LogErrorAndRemoveClient(process, ex);
					}
				}

				return false;
			}
		}

		/// <summary>Enumerates all pipes started by the ReClass.NET PipeServer.</summary>
		/// <param name="callbackProcess">The callback which gets called for every process.</param>
		[DllExport(CallingConvention = CallingConvention.StdCall)]
		private static void EnumerateProcesses(EnumerateProcessCallback callbackProcess)
		{
			if (callbackProcess == null)
			{
				return;
			}

			foreach (var pipe in GetPipes())
			{
				var platform = new DirectoryInfo(pipe).Parent?.Name ?? string.Empty;
#if WIN64
				if (platform.ToLower() == "x64")
#else
				if (platform.ToLower() == "x86")
#endif
				{
					var data = new EnumerateProcessData
					{
						Id = (IntPtr)pipe.GetHashCode(),
						Path = pipe
					};

					callbackProcess(ref data);
				}
			}
		}

		/// <summary>Enumerate all sections and modules of the remote process.</summary>
		/// <remarks><![CDATA[
		/// Protocoll:
		/// -> EnumerateRemoteSectionsAndModulesMessage
		/// <- EnumerateRemoteSectionCallbackMessage [*]
		/// <- EnumerateRemoteModuleCallbackMessage [*]
		/// <- StatusMessage
		/// 
		/// Both callback messages can arrive in random order and count. The enumeration is finished if the StatusMessage was received.
		/// ]]></remarks>
		/// <param name="process">The process.</param>
		/// <param name="callbackSection">The callback which gets called for every section.</param>
		/// <param name="callbackModule">The callback which gets called for every module.</param>
		[DllExport(CallingConvention = CallingConvention.StdCall)]
		private static void EnumerateRemoteSectionsAndModules(IntPtr process, EnumerateRemoteSectionCallback callbackSection, EnumerateRemoteModuleCallback callbackModule)
		{
			if (callbackSection == null && callbackModule == null)
			{
				return;
			}

			lock (sync)
			{
				var client = GetClientById(process);
				if (client != null)
				{
					try
					{
						client.Send(new EnumerateRemoteSectionsAndModulesMessage());

						while (true)
						{
							var message = client.Receive();
							if (message is StatusMessage)
							{
								break;
							}

							var callbackSectionMessage = message as EnumerateRemoteSectionCallbackMessage;
							if (callbackSectionMessage != null)
							{
								var data = new EnumerateRemoteSectionData
								{
									BaseAddress = callbackSectionMessage.BaseAddress,
									Size = callbackSectionMessage.Size,
									Type = callbackSectionMessage.Type,
									Protection = callbackSectionMessage.Protection,
									Name = callbackSectionMessage.Name,
									ModulePath = callbackSectionMessage.ModulePath
								};

								callbackSection?.Invoke(ref data);
							}

							var callbackModuleMessage = message as EnumerateRemoteModuleCallbackMessage;
							if (callbackModuleMessage != null)
							{
								var data = new EnumerateRemoteModuleData
								{
									BaseAddress = callbackModuleMessage.BaseAddress,
									Size = callbackModuleMessage.Size,
									Path = callbackModuleMessage.Path
								};

								callbackModule?.Invoke(ref data);
							}
						}
					}
					catch (Exception ex)
					{
						LogErrorAndRemoveClient(process, ex);
					}
				}
			}
		}
	}
}
