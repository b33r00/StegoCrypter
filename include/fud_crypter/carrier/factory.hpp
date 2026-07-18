#pragma once
#include "icarrier.hpp"
#include "fud_crypter/config.hpp"
#include "fud_crypter/logging/logger.hpp"
#include <memory>
#include <vector>

namespace fud_crypter {
    namespace carrier {

        enum class FileType { PDF, PNG, GIF, UNKNOWN };

        FileType detect_file_type(const std::vector<uint8_t>& data);

        std::unique_ptr<ICarrier> create_carrier(
            const std::string& path,
            const LoaderConfig& config,
            std::shared_ptr<Logger> logger
        );

    } // namespace carrier
} // namespace fud_crypter
