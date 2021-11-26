# ForkPlayground
A library to implement the Process Forking attack described in this blog post. 

**ForkLib** - C++ library that implements the Process Forking attack.
**ForkDump** - Proof-of-Concept application to create a memory dump of an arbitrary process using the ForkLib.

## Usage
```
ForkDump.exe [dump file name] [target process ID]
```
**dump file name** - The name of the file to write the dump of the process to.
**target process ID** - The process ID of the process whose memory the tool should dump.

## Download
[ForkDump x64](https://github.com/D4stiny/ForkPlayground/releases/download/1.0.0/ForkDump-x64.exe)
[ForkDump x86](https://github.com/D4stiny/ForkPlayground/releases/download/1.0.0/ForkDump-x86.exe)