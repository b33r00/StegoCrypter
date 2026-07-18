#pragma once
#include "icarrier.hpp"
#include <vector>

namespace fud_crypter {
    namespace carrier {

        class GifCarrier : public ICarrier {
        public:
            explicit GifCarrier(std::vector<uint8_t> data);
            std::vector<uint8_t> extract_payload() override;
            std::string format_name() const override { return "GIF"; }
            std::string description() const override;
            size_t file_size() const override { return data_.size(); }

        private:
            std::vector<uint8_t> data_;
        };

    } // namespace carrier
} // namespace fud_crypter
