#pragma once
#include <vector>
#include <cstdint>
#include <string>

namespace fud_crypter {
    namespace crypto {

        class ICrypto {
        public:
            virtual ~ICrypto() = default;
            virtual std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data) = 0;
            virtual std::string algorithm_name() const = 0;
            virtual size_t key_size() const = 0;
        };

    } // namespace crypto
} // namespace fud_crypter
