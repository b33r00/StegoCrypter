#pragma once
#include "icrypto.hpp"
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

namespace fud_crypter {
    namespace crypto {

        enum class Algorithm {
            XOR,
            AES_256_GCM
        };

        std::unique_ptr<ICrypto> create_decryptor(const std::string& key_path);
        std::unique_ptr<ICrypto> create_decryptor(std::vector<uint8_t> key);

    } // namespace crypto
} // namespace fud_crypter
