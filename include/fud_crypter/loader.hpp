#pragma once
#include "fud_crypter/config.hpp"
#include "fud_crypter/logging/logger.hpp"
#include <memory>

namespace fud_crypter {

    class Loader {
    public:
        explicit Loader(const LoaderConfig& config);
        bool execute();

    private:
        LoaderConfig config_;
        std::shared_ptr<Logger> logger_;
    };

} // namespace fud_crypter
