# Building Carrier Engine

## Prerequisites

### Windows
- Visual Studio 2019 or later (with C++ workload)
- OR MinGW-w64 (GCC 8+)
- CMake 3.16 or later

### Linux
- GCC 8+ or Clang 10+
- CMake 3.16 or later
- make or ninja

## Build Instructions

### Using CMake (Recommended)

```bash
# Clone or navigate to the project
cd carrier-engine

# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build . --config Release

# The executable will be in:
#   Windows: Release/carrier-engine.exe
#   Linux:   carrier-engine
