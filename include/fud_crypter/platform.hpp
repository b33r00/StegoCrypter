#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>

namespace fud_crypter {
    namespace platform {

        using ThreadFunction = void(*)(void*);

        struct ThreadHandle {
            #ifdef FUD_PLATFORM_WINDOWS
            void* handle;
            static ThreadHandle invalid() { return {nullptr}; }
            #else
            unsigned long handle;
            static ThreadHandle invalid() { return {0}; }
            #endif
        };

        void* allocate_executable_memory(size_t size);
        void free_executable_memory(void* ptr, size_t size);
        void make_memory_executable(void* ptr, size_t size);

        ThreadHandle create_thread(ThreadFunction func, void* arg);
        void wait_for_thread(ThreadHandle handle);
        void close_thread(ThreadHandle handle);

        std::vector<uint8_t> read_file(const std::string& path);
        void write_file(const std::string& path, const std::vector<uint8_t>& data);

        std::string platform_name();
        std::string last_error_message();

    } // namespace platform
} // namespace fud_crypter
