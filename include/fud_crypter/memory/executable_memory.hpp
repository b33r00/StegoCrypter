#pragma once

#include "fud_crypter/platform.hpp"
#include <cstddef>
#include <vector>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace fud_crypter {
    namespace memory {

        class ExecutableMemory {
        public:
            explicit ExecutableMemory(size_t size)
            : size_(size), memory_(platform::allocate_executable_memory(size)) {}

            ~ExecutableMemory() {
                if (memory_) {
                    platform::free_executable_memory(memory_, size_);
                }
            }

            ExecutableMemory(const ExecutableMemory&) = delete;
            ExecutableMemory& operator=(const ExecutableMemory&) = delete;

            ExecutableMemory(ExecutableMemory&& other) noexcept
            : size_(other.size_), memory_(other.memory_) {
                other.memory_ = nullptr;
                other.size_ = 0;
            }

            void write(const std::vector<uint8_t>& data) {
                if (data.size() > size_) {
                    throw std::runtime_error("Data exceeds memory size");
                }
                std::memcpy(memory_, data.data(), data.size());
            }

            void make_executable() {
                platform::make_memory_executable(memory_, size_);
            }

            void* ptr() noexcept { return memory_; }
            size_t size() const noexcept { return size_; }

        private:
            size_t size_;
            void* memory_;
        };

    } // namespace memory
} // namespace fud_crypter
