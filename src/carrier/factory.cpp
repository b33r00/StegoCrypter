#include "fud_crypter/carrier/factory.hpp"
#include "fud_crypter/carrier/pdf_carrier.hpp"
#include "fud_crypter/carrier/gif_carrier.hpp"
#include "fud_crypter/carrier/png_carrier.hpp"
#include <fstream>
#include <cstring>
#include <stdexcept>

namespace fud_crypter {
    namespace carrier {

        FileType detect_file_type(const std::vector<uint8_t>& data) {
            if (data.size() < 8) return FileType::UNKNOWN;
            if (std::memcmp(data.data(), "%PDF-", 5) == 0) return FileType::PDF;
            if (std::memcmp(data.data(), "GIF8", 4) == 0) return FileType::GIF;
            const uint8_t png_sig[8] = {137,80,78,71,13,10,26,10};
            if (std::memcmp(data.data(), png_sig, 8) == 0) return FileType::PNG;
            return FileType::UNKNOWN;
        }

        std::unique_ptr<ICarrier> create_carrier(
            const std::string& path,
            const LoaderConfig& config,
            std::shared_ptr<Logger> logger
        ) {
            (void)config; // unused
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            if (!file) throw std::runtime_error("Cannot open carrier: " + path);
            auto size = file.tellg();
            if (size <= 0) throw std::runtime_error("Carrier file is empty: " + path);
            std::vector<uint8_t> data(static_cast<size_t>(size));
            file.seekg(0);
            file.read(reinterpret_cast<char*>(data.data()), size);

            FileType type = detect_file_type(data);
            switch (type) {
                case FileType::PDF:
                    logger->info("Carrier: PDF");
                    return std::make_unique<PdfCarrier>(std::move(data));
                case FileType::GIF:
                    logger->info("Carrier: GIF");
                    return std::make_unique<GifCarrier>(std::move(data));
                case FileType::PNG:
                    logger->info("Carrier: PNG");
                    return std::make_unique<PngCarrier>(std::move(data));
                default:
                    throw std::runtime_error("Unsupported carrier format");
            }
        }

    } // namespace carrier
} // namespace fud_crypter
