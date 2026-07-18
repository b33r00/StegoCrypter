#include "fud_crypter/carrier/pdf_carrier.hpp"
#include <stdexcept>
#include <cstring>
#include <cctype>

namespace fud_crypter {
    namespace carrier {

        static size_t find_bytes(const std::vector<uint8_t>& hay, const char* needle,
                                 size_t needle_len, size_t from = 0) {
            if (needle_len == 0 || hay.size() < needle_len) return std::string::npos;
            for (size_t i = from; i <= hay.size() - needle_len; ++i)
                if (std::memcmp(hay.data() + i, needle, needle_len) == 0) return i;
                return std::string::npos;
                                 }

                                 static bool dict_find_int(const std::string& d, const std::string& key, uint64_t& v) {
                                     size_t p = d.find(key);
                                     if (p == std::string::npos) return false;
                                     p += key.size();
                                     while (p < d.size() && std::isspace(d[p])) ++p;
                                     if (p >= d.size() || !std::isdigit(d[p])) return false;
                                     v = 0;
                                     while (p < d.size() && std::isdigit(d[p]))
                                         v = v * 10 + (d[p++] - '0');
                                     return true;
                                 }

                                 PdfCarrier::PdfCarrier(std::vector<uint8_t> data) : data_(std::move(data)) {}

                                 std::string PdfCarrier::description() const {
                                     return "PDF document, " + std::to_string(data_.size()) + " bytes";
                                 }

                                 uint64_t PdfCarrier::find_highest_object_number() const {
                                     uint64_t max = 0;
                                     size_t pos = 0;
                                     while ((pos = find_bytes(data_, " obj", 4, pos)) != std::string::npos) {
                                         size_t p = pos;
                                         while (p > 0 && std::isdigit(data_[p-1])) --p;
                                         uint64_t num = 0;
                                         while (p < pos && std::isdigit(data_[p]))
                                             num = num * 10 + (data_[p++] - '0');
                                         if (num > max) max = num;
                                         pos += 4;
                                     }
                                     return max;
                                 }

                                 std::vector<uint8_t> PdfCarrier::extract_payload() {
                                     uint64_t obj_num = find_highest_object_number();
                                     if (obj_num == 0) throw std::runtime_error("No objects found in PDF");

                                     std::string target = std::to_string(obj_num) + " 0 obj";
                                     size_t obj_off = find_bytes(data_, target.c_str(), target.size(), 0);
                                     if (obj_off == std::string::npos)
                                         throw std::runtime_error("Object not found");

                                     size_t stream_start = find_bytes(data_, "stream", 6, obj_off);
                                     if (stream_start == std::string::npos)
                                         throw std::runtime_error("Object has no stream");

                                     std::string dict(data_.begin() + obj_off, data_.begin() + stream_start);
                                     uint64_t length = 0;
                                     if (!dict_find_int(dict, "/Length", length))
                                         throw std::runtime_error("Cannot find /Length");

                                     size_t data_start = stream_start + 6;
                                     while (data_start < data_.size() && (data_[data_start] == '\r' || data_[data_start] == '\n'))
                                         ++data_start;

                                     if (data_start + length > data_.size())
                                         throw std::runtime_error("Stream truncated");

                                     return std::vector<uint8_t>(data_.begin() + data_start,
                                                                 data_.begin() + data_start + length);
                                 }

    } // namespace carrier
} // namespace fud_crypter
