#pragma once
#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>

namespace fud_crypter {
    namespace api {

        // ROR13 hash (a Windows API hashing szabványa)
        inline uint32_t ror13_hash(const char* str) {
            uint32_t hash = 0;
            while (*str) {
                hash = (hash >> 13) | (hash << (32 - 13));
                hash ^= static_cast<uint32_t>(*str);
                ++str;
            }
            return hash;
        }

        // Dinamikus API feloldó osztály
        class ApiResolver {
        public:
            ApiResolver();

            // Függvény feloldása hash alapján
            FARPROC resolve(uint32_t hash) const;

            // Template helper a típusos hívásokhoz
            template<typename T>
            T resolve(uint32_t hash) const {
                return reinterpret_cast<T>(resolve(hash));
            }

        private:
            HMODULE h_ntdll_;
            HMODULE h_kernel32_;
        };

        // Globális resolver példány (inline, hogy ne kelljen linker)
        inline ApiResolver g_api;

    } // namespace api
} // namespace fud_crypter
