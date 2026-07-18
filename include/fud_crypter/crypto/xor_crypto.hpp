#pragma once
#include "icrypto.hpp"
#include <vector>
#include <cstdint>
#include <stdexcept>

namespace fud_crypter {
    namespace crypto {

        // Szabadon álló függvény a builder-ek számára
        std::vector<uint8_t> xor_cipher(const std::vector<uint8_t>& data,
                                        const std::vector<uint8_t>& key);

        class XorCrypto : public ICrypto {
        public:
            explicit XorCrypto(std::vector<uint8_t> key);
            std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data) override;
            std::string algorithm_name() const override;
            size_t key_size() const override;

        private:
            std::vector<uint8_t> key_;
        };

    } // namespace crypto
} // namespace fud_crypter
