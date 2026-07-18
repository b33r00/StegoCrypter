#pragma once
#include "icarrier.hpp"
#include <vector>
#include <cstdint>

namespace fud_crypter {
    namespace carrier {

        class PdfCarrier : public ICarrier {
        public:
            explicit PdfCarrier(std::vector<uint8_t> data);
            std::vector<uint8_t> extract_payload() override;
            std::string format_name() const override { return "PDF"; }
            std::string description() const override;
            size_t file_size() const override { return data_.size(); }

        private:
            std::vector<uint8_t> data_;
            uint64_t find_highest_object_number() const;
        };

    } // namespace carrier
} // namespace fud_crypter
