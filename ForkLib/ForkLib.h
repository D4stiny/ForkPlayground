/*
 * FORKLIB - Process Snapshotting via forking.
 *
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE', which is part of this source code package.
 *
 * COPYRIGHT Bill Demirkapi 2021
 */
#pragma once
#include "pch.h"

class ForkSnapshot
{
	/// <summary>
	/// A handle to the target process we're forking. This handle should hold the PROCESS_CREATE_PROCESS right.
	/// </summary>
	HANDLE TargetProcess;
	/// <summary>
	/// Handle to the current snapshot process.
	/// </summary>
	HANDLE CurrentSnapshotProcess;
public:
	ForkSnapshot (
		_In_ HANDLE TargetProcess
		);
	ForkSnapshot (
		_In_ DWORD TargetProcessId
		);
	~ForkSnapshot (
		VOID
		);

	HANDLE TakeSnapshot (
		VOID
		);
	BOOL CleanSnapshot (
		VOID
		);
};