/*
 * FORKLIB - Process Snapshotting via forking.
 *
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE', which is part of this source code package.
 *
 * COPYRIGHT Bill Demirkapi 2021
 */
#include "pch.h"
#include "ForkLib.h"

/// <summary>
/// Initialize a fork snapshot. Attempts to open a PROCESS_CREATE_PROCESS handle to the target.
/// </summary>
/// <param name="TargetProcessId">The process ID of the target process to fork.</param>
ForkSnapshot::ForkSnapshot (
	_In_ DWORD TargetProcessId
	)
{
	this->CurrentSnapshotProcess = NULL;
	this->TargetProcess = OpenProcess(PROCESS_CREATE_PROCESS, FALSE, TargetProcessId);
	if (this->TargetProcess == NULL)
	{
		DBGPRINT("[FORKLIB] Failed to open a PROCESS_CREATE_PROCESS handle to target process %i with last error %i.", TargetProcessId, GetLastError());
	}
}

/// <summary>
/// Initialize a fork snapshot.
/// </summary>
/// <param name="TargetProcess">The process ID of the target process to fork.</param>
ForkSnapshot::ForkSnapshot (
	_In_ HANDLE TargetProcess
	)
{
	this->CurrentSnapshotProcess = NULL;
	this->TargetProcess = TargetProcess;
}

/// <summary>
/// Deconstruct a fork snapshot. This will clean up any open snapshots.
/// </summary>
ForkSnapshot::~ForkSnapshot (
	VOID
	)
{
	if (this->CurrentSnapshotProcess != NULL)
	{
		this->CleanSnapshot();
	}
}

/// <summary>
/// Take a snapshot of the target process.
/// </summary>
/// <returns>A PROCESS_ALL_ACCESS HANDLE to a snapshot of the target process.</returns>
HANDLE
ForkSnapshot::TakeSnapshot (
	VOID
	)
{
	NTSTATUS status;

	//
	// If we have an existing snapshot process, we need to clean it up before creating another one.
	//
	if (this->CurrentSnapshotProcess != NULL)
	{
		if (this->CleanSnapshot() == FALSE)
		{
			DBGPRINT("[FORKLIB] Failed to clean up existing snapshot before taking another one.");
			return NULL;
		}
	}

	//
	// Attempt to create a fork of the target process.
	//
	status = NtCreateProcessEx(&this->CurrentSnapshotProcess,
							   PROCESS_ALL_ACCESS,
							   NULL,
							   this->TargetProcess,
							   0,
							   NULL,
							   NULL,
							   NULL,
							   0);
	if (NT_SUCCESS(status) == FALSE)
	{
		DBGPRINT("[FORKLIB] Failed to create fork process with status 0x%X.", status);
		return NULL;
	}

	//
	// If the previous call succeeded, we have created a fork/snapshot of the target process.
	// Yes, it really is that simple.
	//
	return this->CurrentSnapshotProcess;
}

/// <summary>
/// Attempts to clean up the current snapshot by terminating the forked process.
/// </summary>
/// <returns>TRUE if clean up successful, FALSE otherwise.</returns>
BOOL
ForkSnapshot::CleanSnapshot (
	VOID
	)
{
	BOOL cleanSuccess;

	cleanSuccess = TRUE;

	if (this->CurrentSnapshotProcess)
	{
		cleanSuccess = TerminateProcess(this->CurrentSnapshotProcess, 0);
		CloseHandle(this->CurrentSnapshotProcess);
		if (cleanSuccess == FALSE)
		{
			DBGPRINT("[FORKLIB] Failed to terminate process with last error %i.", GetLastError());
		}
		this->CurrentSnapshotProcess = NULL;
	}
	
	return cleanSuccess;
}