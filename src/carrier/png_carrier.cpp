#include "fud_crypter/carrier/png_carrier.hpp"
#include <cstring>
#include <stdexcept>
#include <optional>

namespace fud_crypter {
    namespace carrier {

        static uint32_t be32(const std::vector<uint8_t>& b, size_t p) {
            return (static_cast<uint32_t>(b[p]) << 24) | (b[p+1] << 16) |
            (b[p+2] << 8) | b[p+3];
        }

        static bool is_png(const std::vector<uint8_t>& b) {
            static const uint8_t sig[8] = {137,80,78,71,13,10,26,10};
            return b.size() >= 8 && std::memcmp(b.data(), sig, 8) == 0;
        }

        static std::optional<std::vector<uint8_t>> base64_decode(const std::string& in) {
            auto val = [](char c) -> int {
                if (c >= 'A' && c <= 'Z') return c - 'A';
                if (c >= 'a' && c <= 'z') return c - 'a' + 26;
                if (c >= '0' && c <= '9') return c - '0' + 52;
                if (c == '+') return 62;
                if (c == '/') return 63;
                return -1;
            };
            std::vector<uint8_t> out;
            uint32_t buf = 0;
            int bits = 0;
            for (char c : in) {
                if (c == '=') break;
                if (c == '\n' || c == '\r' || c == ' ') continue;
                int v = val(c);
                if (v < 0) return std::nullopt;
                buf = (buf << 6) | static_cast<uint32_t>(v);
                bits += 6;
                if (bits >= 8) {
                    bits -= 8;
                    out.push_back(static_cast<uint8_t>((buf >> bits) & 0xFF));
                }
            }
            return out;
        }

        PngCarrier::PngCarrier(std::vector<uint8_t> data) : data_(std::move(data)) {}

        std::string PngCarrier::description() const {
            return "PNG image, " + std::to_string(data_.size()) + " bytes";
        }

        std::vector<uint8_t> PngCarrier::extract_payload() {
            if (!is_png(data_)) throw std::runtime_error("Not a PNG");

            size_t pos = 8;
            while (pos + 8 <= data_.size()) {
                uint32_t len = be32(data_, pos);
                std::string type(data_.begin()+pos+4, data_.begin()+pos+8);
                size_t data_start = pos + 8;
                if (type == "iTXt") {
                    size_t p = data_start;
                    size_t end = data_start + len;
                    size_t kw_start = p;
                    while (p < end && data_[p] != 0) ++p;
                    std::string kw(data_.begin() + kw_start, data_.begin() + p);
                    if (kw == "pdmeta-payload") {
                        p += 1;
                        p += 2;
                        while (p < end && data_[p] != 0) ++p;
                        if (p < end) ++p;
                        while (p < end && data_[p] != 0) ++p;
                        if (p < end) ++p;
                        std::string text(data_.begin() + p, data_.begin() + end);
                        auto decoded = base64_decode(text);
                        if (!decoded) throw std::runtime_error("Invalid base64 in iTXt");
                        return *decoded;
                    }
                }
                pos = data_start + len + 4;
                if (type == "IEND") break;
            }
            throw std::runtime_error("No matching iTXt chunk found");
        }

    } // namespace carrier
} // namespace fud_crypter
