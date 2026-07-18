#include "fud_crypter/crypto/xor_crypto.hpp"
#include <vector>
#include <cstdint>

namespace fud_crypter {
    namespace crypto {

        std::vector<uint8_t> xor_cipher(const std::vector<uint8_t>& data,
                                        const std::vector<uint8_t>& key) {
            if (key.empty()) return data;
            std::vector<uint8_t> out(data.size());
            for (size_t i = 0; i < data.size(); ++i)
                out[i] = data[i] ^ key[i % key.size()];
            return out;
                                        }

                                        XorCrypto::XorCrypto(std::vector<uint8_t> key) : key_(std::move(key)) {
                                            if (key_.empty()) throw std::runtime_error("XOR key cannot be empty");
                                        }

                                        std::vector<uint8_t> XorCrypto::decrypt(const std::vector<uint8_t>& data) {
                                            return xor_cipher(data, key_);
                                        }

                                        std::string XorCrypto::algorithm_name() const { return "XOR"; }
                                        size_t XorCrypto::key_size() const { return key_.size(); }

    } // namespace crypto
} // namespace fud_crypter
