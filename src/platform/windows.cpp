#include "fud_crypter/platform.hpp"
#include <windows.h>
#include <fstream>
#include <stdexcept>

namespace fud_crypter {
    namespace platform {

        void* allocate_executable_memory(size_t size) {
            void* ptr = VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (!ptr) {
                throw std::runtime_error("VirtualAlloc failed: " + last_error_message());
            }
            return ptr;
        }

        void free_executable_memory(void* ptr, size_t /*size*/) {
            if (ptr) VirtualFree(ptr, 0, MEM_RELEASE);
        }

        void make_memory_executable(void* ptr, size_t size) {
            DWORD old;
            if (!VirtualProtect(ptr, size, PAGE_EXECUTE_READ, &old)) {
                throw std::runtime_error("VirtualProtect failed: " + last_error_message());
            }
        }

        ThreadHandle create_thread(ThreadFunction func, void* arg) {
            HANDLE h = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)func, arg, 0, nullptr);
            if (!h) {
                throw std::runtime_error("CreateThread failed: " + last_error_message());
            }
            ThreadHandle handle;
            handle.handle = h;
            return handle;
        }

        void wait_for_thread(ThreadHandle handle) {
            WaitForSingleObject(handle.handle, INFINITE);
        }

        void close_thread(ThreadHandle handle) {
            if (handle.handle) CloseHandle(handle.handle);
        }

        std::vector<uint8_t> read_file(const std::string& path) {
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            if (!file) return {};
            auto size = file.tellg();
            if (size <= 0) return {};
            std::vector<uint8_t> buf(static_cast<size_t>(size));
            file.seekg(0);
            file.read(reinterpret_cast<char*>(buf.data()), size);
            return buf;
        }

        void write_file(const std::string& path, const std::vector<uint8_t>& data) {
            std::ofstream file(path, std::ios::binary | std::ios::trunc);
            if (!file) throw std::runtime_error("Cannot write file: " + path);
            file.write(reinterpret_cast<const char*>(data.data()), data.size());
        }

        std::string platform_name() { return "Windows"; }

        std::string last_error_message() {
            DWORD err = GetLastError();
            if (err == 0) return "Success";
            char* msg = nullptr;
            FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                           nullptr, err, 0, (LPSTR)&msg, 0, nullptr);
            std::string result(msg ? msg : "Unknown error");
            LocalFree(msg);
            return result;
        }

    } // namespace platform
} // namespace fud_crypter
