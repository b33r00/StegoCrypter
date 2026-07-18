#include "fud_crypter/carrier/gif_carrier.hpp"
#include <cstring>
#include <stdexcept>

namespace fud_crypter {
    namespace carrier {

        constexpr char kGifAppId[8] = {'P','D','M','E','T','A','0','1'};
        constexpr char kGifAppAuth[3] = {'B','I','N'};

        static size_t gif_skip_subblocks(const std::vector<uint8_t>& b, size_t pos) {
            while (pos < b.size()) {
                uint8_t len = b[pos++];
                if (len == 0) return pos;
                pos += len;
            }
            return std::string::npos;
        }

        GifCarrier::GifCarrier(std::vector<uint8_t> data) : data_(std::move(data)) {}

        std::string GifCarrier::description() const {
            return "GIF image, " + std::to_string(data_.size()) + " bytes";
        }

        std::vector<uint8_t> GifCarrier::extract_payload() {
            size_t pos = 0;
            while (pos < data_.size()) {
                if (data_[pos] == 0x21 && pos + 14 <= data_.size() &&
                    data_[pos+1] == 0xFF && data_[pos+2] == 0x0B &&
                    std::memcmp(data_.data() + pos + 3, kGifAppId, 8) == 0 &&
                    std::memcmp(data_.data() + pos + 11, kGifAppAuth, 3) == 0) {
                    size_t p = pos + 14;
                std::vector<uint8_t> out;
                while (p < data_.size()) {
                    uint8_t len = data_[p++];
                    if (len == 0) break;
                    if (p + len > data_.size())
                        throw std::runtime_error("Truncated sub-block");
                    out.insert(out.end(), data_.begin() + p, data_.begin() + p + len);
                    p += len;
                }
                return out;
                    }
                    if (data_[pos] == 0x21) {
                        size_t p = gif_skip_subblocks(data_, pos + 2);
                        if (p == std::string::npos) break;
                        pos = p;
                    } else {
                        ++pos;
                    }
            }
            throw std::runtime_error("No matching Application Extension found");
        }

    } // namespace carrier
} // namespace fud_crypter
