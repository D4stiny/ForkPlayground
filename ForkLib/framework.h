#pragma once
#include <Windows.h>
#include <winternl.h>

#define RCAST reinterpret_cast
#define SCAST static_cast
#define CCAST const_cast

#ifdef _DEBUG
#include <cstdio>
#define DBGPRINT(msg, ...) printf(msg"\n", __VA_ARGS__)
#else
#define DBGPRINT(x, ...)
#endif

#define PROCESS_CREATE_FLAGS_INHERIT_HANDLES 0x00000004

#pragma comment(lib, "ntdll.lib")

extern "C"
{
    NTSYSCALLAPI
    NTSTATUS
    NTAPI
    NtCreateProcessEx (
        _Out_ PHANDLE ProcessHandle,
        _In_ ACCESS_MASK DesiredAccess,
        _In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
        _In_ HANDLE ParentProcess,
        _In_ ULONG Flags,
        _In_opt_ HANDLE SectionHandle,
        _In_opt_ HANDLE DebugPort,
        _In_opt_ HANDLE ExceptionPort,
        _In_ ULONG JobMemberLevel
        );
}
