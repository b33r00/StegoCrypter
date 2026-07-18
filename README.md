# CarrierEngine

A modular C++17 framework for embedding and extracting encrypted payloads into common file formats (PDF, PNG, GIF) with polymorphic stub generation.

## Features
- **PDF / PNG / GIF** carrier support
- **XOR encryption** with random key generation
- **Polymorphic stub** generation (`--polymorph`)
- **Cross-platform** (Linux build + Windows cross-compile)
- **Advanced evasion**: API hashing, direct syscalls, process hollowing (Windows)

## Build
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
