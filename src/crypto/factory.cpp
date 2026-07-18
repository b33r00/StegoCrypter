#include "fud_crypter/crypto/factory.hpp"
#include "fud_crypter/crypto/xor_crypto.hpp"
#include <fstream>
#include <stdexcept>

namespace fud_crypter {
    namespace crypto {

        std::unique_ptr<ICrypto> create_decryptor(const std::string& key_path) {
            std::ifstream file(key_path, std::ios::binary | std::ios::ate);
            if (!file) {
                throw std::runtime_error("Cannot open key: " + key_path);
            }

            auto size = file.tellg();
            if (size <= 0) {
                throw std::runtime_error("Key file is empty or invalid: " + key_path);
            }

            std::vector<uint8_t> key(static_cast<size_t>(size));
            file.seekg(0);
            file.read(reinterpret_cast<char*>(key.data()), size);

            return std::make_unique<XorCrypto>(std::move(key));
        }

        std::unique_ptr<ICrypto> create_decryptor(std::vector<uint8_t> key) {
            return std::make_unique<XorCrypto>(std::move(key));
        }

    } // namespace crypto
} // namespace fud_crypter
