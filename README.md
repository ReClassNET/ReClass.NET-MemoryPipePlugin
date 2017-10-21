# ReClass.NET MemoryPipe Plugin
A ReClass.NET plugin which allows direct memory access via dll injection.  
The injected dll works like a server which reads requests from a pipe. Results are transfered back to ReClass.NET through the pipe.

## Installation
- Download from https://github.com/KN4CK3R/ReClass.NET-MemoryPipePlugin
- Copy the dll files in the appropriate Plugin folder (ReClass.NET/x86/Plugins or ReClass.NET/x64/Plugins)
- Start ReClass.NET and check the plugins form if the MemoryPipe plugin is listed. Open the "Native" tab and switch all available methods to the MemoryPipe plugin.
- Inject the PipeServer.dll into the target.
- Attach to the process via its pipe and use ReClass.NET as normal. (Note: Controlling the remote process (start, stop, pause) is not supported.)

![](https://abload.de/img/pipe0mj0z.jpg)

## Compiling
If you want to compile the ReClass.NET MemoryPipe Plugin just fork the repository and create the following folder structure. If you don't use this structure you need to fix the project references.

```
..\ReClass.NET\
..\ReClass.NET\ReClass.NET\ReClass.NET.csproj
..\ReClass.NET-MemoryPipePlugin
..\ReClass.NET-MemoryPipePlugin\Plugin
..\ReClass.NET-MemoryPipePlugin\Plugin\MemoryPipePlugin.csproj
```