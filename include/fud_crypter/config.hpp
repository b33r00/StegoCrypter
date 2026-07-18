#pragma once
#include <string>
#include <cstdint>
#include <vector>
#include "fud_crypter/logging/logger.hpp"

namespace fud_crypter {

    struct CarrierConfig {
        uint64_t pdf_object_number = 0;
    };

    struct BuilderConfig {
        std::string input_file;      // Payload fájl (shellcode, dll, exe)
        std::string output_file;     // Kimeneti carrier (pdf/gif/png) vagy stub.exe
        std::string format;          // "pdf", "gif", "png", "exe"
        std::string key_file;        // Kulcs fájl (ha üres, automatikus generálás)
        bool polymorph = false;      // Polimorf stub generálás
        bool execute = false;        // Embed után azonnal futtasd
        bool verbose = false;        // Részletes log
    };

    struct LoaderConfig {
        std::string carrier_file;    // Carrier fájl (pdf/gif/png)
        std::string key_file;        // Kulcs fájl
        bool verbose = false;
    };

} // namespace fud_crypter
