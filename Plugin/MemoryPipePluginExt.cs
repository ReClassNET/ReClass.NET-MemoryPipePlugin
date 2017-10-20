using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.Drawing;
using System.IO;
using System.IO.Pipes;
using System.Linq;
using ReClassNET.Core;
using ReClassNET.Plugins;

namespace MemoryPipePlugin
{
	public partial class MemoryPipePluginExt : Plugin, ICoreProcessFunctions
	{
		private const string PipePrefix = @"\\.\pipe\";

		private readonly object sync = new object();

		private IPluginHost host;

		private Dictionary<IntPtr, MessageClient> openPipes;

		public override Image Icon => Properties.Resources.logo;

		public override bool Initialize(IPluginHost host)
		{
			Contract.Requires(host != null);

			//System.Diagnostics.Debugger.Launch();

			if (this.host != null)
			{
				Terminate();
			}

			this.host = host ?? throw new ArgumentNullException(nameof(host));

			host.Process.CoreFunctions.RegisterFunctions("Memory Pipe", this);

			openPipes = new Dictionary<IntPtr, MessageClient>();

			return true;
		}

		public override void Terminate()
		{
			foreach (var client in openPipes.Values)
			{
				client.Dispose();
			}
			openPipes.Clear();

			host = null;
		}

		/// <summary>Gets a <see cref="MessageClient"/> by its plugin internal identifier.</summary>
		/// <param name="id">The identifier.</param>
		/// <returns>The client or null if the identifier doesn't exist.</returns>
		private MessageClient GetClientById(IntPtr id)
		{
			openPipes.TryGetValue(id, out var client);
			return client;
		}

		/// <summary>Logs the exception and removes client.</summary>
		/// <param name="id">The identifier.</param>
		/// <param name="ex">The exception.</param>
		private void LogErrorAndRemoveClient(IntPtr id, Exception ex)
		{
			Contract.Requires(ex != null);

			GetClientById(id)?.Dispose();

			openPipes.Remove(id);

			host.Logger.Log(ex);
		}

		/// <summary>Enumerates all pipes created by the ReClass.NET PipeServer.</summary>
		/// <returns>An enumerator to all pipes.</returns>
		private static IEnumerable<string> GetPipes()
		{
			return Directory.GetFiles(PipePrefix).Where(p => p.Contains("ReClass.NET"));
		}

		/// <summary>Creates a <see cref="MessageClient"/> for the given pipe.</summary>
		/// <param name="pipePath">Full pathname of the pipe.</param>
		/// <returns>The <see cref="MessageClient"/> for the pipe.</returns>
		private MessageClient CreateClientForPipe(string pipePath)
		{
			var pipeName = pipePath.Substring(PipePrefix.Length);

			var pipe = new NamedPipeClientStream(".", pipeName, PipeDirection.InOut);
			pipe.Connect();
			pipe.ReadMode = PipeTransmissionMode.Message;

			var client = new MessageClient(pipe);

			client.RegisterMessage<StatusResponse>();
			client.RegisterMessage<ReadMemoryResponse>();
			client.RegisterMessage<EnumerateRemoteSectionResponse>();
			client.RegisterMessage<EnumerateRemoteModuleResponse>();

			openPipes.Add(client.Id, client);

			return client;
		}
	}
}
