#pragma once
#include <vector>
#include <cstdint>
#include <string>

namespace fud_crypter {
    namespace carrier {

        class ICarrier {
        public:
            virtual ~ICarrier() = default;
            virtual std::vector<uint8_t> extract_payload() = 0;
            virtual std::string format_name() const = 0;
            virtual std::string description() const = 0;
            virtual size_t file_size() const = 0;
        };

    } // namespace carrier
} // namespace fud_crypter
