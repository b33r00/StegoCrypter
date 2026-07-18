#include "fud_crypter/platform.hpp"
#include <sys/mman.h>
#include <pthread.h>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <cerrno>
#include <unistd.h>

namespace fud_crypter {
    namespace platform {

        void* allocate_executable_memory(size_t size) {
            void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (ptr == MAP_FAILED) {
                throw std::runtime_error(std::string("mmap failed: ") + strerror(errno));
            }
            return ptr;
        }

        void free_executable_memory(void* ptr, size_t size) {
            if (ptr) munmap(ptr, size);
        }

        void make_memory_executable(void* ptr, size_t size) {
            if (mprotect(ptr, size, PROT_READ | PROT_EXEC) != 0) {
                throw std::runtime_error(std::string("mprotect failed: ") + strerror(errno));
            }
        }

        ThreadHandle create_thread(ThreadFunction func, void* arg) {
            pthread_t tid;
            int ret = pthread_create(&tid, nullptr, (void*(*)(void*))func, arg);
            if (ret != 0) {
                throw std::runtime_error(std::string("pthread_create failed: ") + strerror(ret));
            }
            ThreadHandle h;
            h.handle = static_cast<unsigned long>(tid);
            return h;
        }

        void wait_for_thread(ThreadHandle handle) {
            pthread_t tid = static_cast<pthread_t>(handle.handle);
            pthread_join(tid, nullptr);
        }

        void close_thread(ThreadHandle /*handle*/) {
            // pthread_t doesn't need explicit cleanup
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

        std::string platform_name() { return "Linux"; }

        std::string last_error_message() {
            return strerror(errno);
        }

    } // namespace platform
} // namespace fud_crypter
