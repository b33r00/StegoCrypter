# CarrierEngine

**A modular C++17 framework for embedding and extracting encrypted payloads into common file formats (PDF, PNG, GIF) with polymorphic stub generation.**

> Educational research tool. Designed for studying file format internals, steganography, Windows memory execution, and defensive EDR/AV evasion techniques in a controlled academic environment.

---

## Table of Contents

- [Overview](#overview)
- [Architecture](#architecture)
- [Supported Carriers](#supported-carriers)
- [Key Features](#key-features)
- [Build Instructions](#build-instructions)
  - [Linux (Native)](#linux-native)
  - [Windows (Cross‑compile)](#windows-crosscompile)
- [Usage Examples](#usage-examples)
  - [Embed a Payload](#1-embed-a-payload)
  - [Extract and Execute](#2-extract-and-execute)
  - [Generate a Polymorphic Stub](#3-generate-a-polymorphic-stub)
- [Advanced Evasion Techniques](#advanced-evasion-techniques)
- [File Format Internals](#file-format-internals)
  - [PDF](#pdf)
  - [PNG](#png)
  - [GIF](#gif)
- [Project Structure](#project-structure)
- [License](#license)
- [Author](#author)

---

## Overview

CarrierEngine is a research-grade tool that demonstrates how arbitrary binary data (shellcode, EXE, DLL, scripts) can be hidden inside common file formats using format‑specific side‑channels:

- **PDF** – EmbeddedFile streams or Image XObjects (JPEG)
- **PNG** – iTXt metadata chunks (Base64‑encoded)
- **GIF** – Application Extension blocks (raw binary sub‑blocks)

The payload is XOR‑encrypted with a random 256‑bit key, and a polymorphic stub can be generated to produce a unique, self‑extracting executable for every build. The project also implements real‑world evasion techniques (API hashing, direct syscalls, process hollowing, sleep obfuscation) to study how modern EDR/AV systems operate.

The framework is structured as a modular, extensible engine – each carrier, crypto primitive, and execution method is a pluggable component.

---

## Architecture

```
+------------------+     +-------------------+     +-------------------+
|   Payload (bin)  |     |    Builder        |     |   Carrier File    |
|   (shellcode,    | --> |  (encrypt, embed) | --> |   (PDF/PNG/GIF)   |
|    exe, dll)     |     +-------------------+     +-------------------+
+------------------+            |                            |
                               |                            |
                               v                            v
+------------------+     +-------------------+     +-------------------+
|   Loader (exe)   | <-- |    Extractor      | <-- |   Carrier File    |
|   (decrypt, run) |     |  (extract, decrypt)|     |   (PDF/PNG/GIF)   |
+------------------+     +-------------------+     +-------------------+
         |
         v
+------------------+
|   Execution      |
|   (VirtualAlloc, |
|    CreateThread, |
|    syscall)      |
+------------------+
```

**Data flow:**

1. Builder reads the payload, encrypts it with a key, and embeds it into the chosen carrier.
2. The carrier file (PDF/PNG/GIF) remains fully valid and opens normally.
3. Loader extracts the encrypted payload, decrypts it, allocates executable memory, and runs it (either in the current process or via process hollowing).

---

## Supported Carriers

| Format | Storage Mechanism | Raw Binary | Encryption | Max Size | Notes |
|--------|-------------------|------------|------------|----------|-------|
| PDF | `/EmbeddedFile` stream | Yes | XOR | Unlimited | JPEG payloads can be stored as displayable `/XObject /Image` |
| PNG | `iTXt` chunk (Base64) | Yes (after decoding) | XOR | Unlimited | ~33% overhead due to Base64 encoding |
| GIF | Application Extension block | Yes | XOR | Unlimited | Split into 255‑byte sub‑blocks (spec‑compliant) |

---

## Key Features

- Multi‑carrier support – PDF, PNG, GIF with automatic format detection.
- XOR encryption – Fixed or random 256‑bit key (CSPRNG).
- Polymorphic stub generation – Every stub has unique variable names, junk code, and key.
- Cross‑platform – Native Linux build + Windows cross‑compile (MinGW).
- Advanced evasion (Windows) – API hashing, direct syscalls, process hollowing, sleep obfuscation.
- RAII memory management – Automatic cleanup with `ExecutableMemory`.
- Modular architecture – Interfaces (`ICarrier`, `ICrypto`) for easy extension.
- Command‑line interface – Simple, consistent flags for build/load operations.

---

## Build Instructions

### Linux (Native)

```bash
# Install dependencies (Debian/Ubuntu/Arch)
sudo apt install cmake g++ make   # or sudo pacman -S cmake gcc make

# Clone and build
cd carrier-engine
mkdir build && cd build
cmake ..
make -j$(nproc)
```

The binary `fud-crypter` will be in `build/`.

### Windows (Cross‑compile with MinGW)

```bash
# Install MinGW and NASM (Arch/Artix)
sudo pacman -S mingw-w64-gcc nasm

# Build with the provided toolchain
cd carrier-engine
mkdir build-win && cd build-win
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw.cmake -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

The Windows executable `fud-crypter.exe` will be in `build-win/`.

---

## Usage Examples

### 1. Embed a Payload

```bash
# Generate a test payload (e.g., MessageBox shellcode)
msfvenom -p windows/x64/messagebox TEXT="Hello" -f raw -o payload.bin

# Embed into PDF with a random key
./fud-crypter --build payload.bin -f pdf -o invoice.pdf -k secret.key

# Embed into PNG with a fixed key
./fud-crypter --build payload.bin -f png -o photo.png -k secret.key

# Embed into GIF
./fud-crypter --build payload.bin -f gif -o animation.gif -k secret.key
```

### 2. Extract and Execute

```bash
# Load the carrier and run the payload
./fud-crypter --load invoice.pdf -k secret.key
```

### 3. Generate a Polymorphic Stub (Standalone EXE)

```bash
# Create a self-contained EXE that decrypts and runs the payload
./fud-crypter --build payload.bin -f exe -o loader.exe --polymorph
```

The generated `loader.exe` is a unique binary with random variable names and junk code – each build produces a different signature.

---

## Advanced Evasion Techniques

CarrierEngine includes research‑grade evasion primitives (Windows only) to study how EDR/AV sensors work:

| Technique | Description | Implementation |
|-----------|-------------|----------------|
| API Hashing | Resolve Windows APIs by ROR13 hash – no IAT imports. | `api_resolver.cpp` |
| Direct Syscalls | Bypass user‑mode hooks by calling `syscall` directly. | `syscall.asm`, `syscall.cpp` |
| Process Hollowing | Inject payload into a suspended legitimate process. | `hollowing.cpp` |
| Sleep Obfuscation | Encrypt memory during sleep, decrypt on wake. | `sleep_obf.cpp` |
| Polymorphic Stub | Random variable names, junk code, and key per build. | `polymorph.cpp` |

These are not intended for malicious use – they are educational examples of how defenders must look beyond static signatures.

---

## File Format Internals

### PDF

- Embedding: Appends a new object (`/EmbeddedFile`) with an incremental update (xref + trailer).
- Extraction: Scans for the highest object number and extracts the stream.
- JPEG: If the payload is a JPEG and not encrypted, it is stored as a displayable `/XObject /Image`.

### PNG

- Embedding: Inserts an `iTXt` chunk (international text) just before `IEND`.
- Encoding: Payload is Base64‑encoded to conform to PNG text‑chunk constraints.
- Extraction: Walks chunks, finds `pdmeta-payload` keyword, decodes Base64.

### GIF

- Embedding: Creates an Application Extension block (`0x21, 0xFF`) with a custom identifier `PDMETA01BIN`.
- Format: Splits payload into 255‑byte sub‑blocks (spec‑compliant).
- Extraction: Scans blocks, reassembles sub‑blocks into the original payload.

---

## Project Structure

```
carrier-engine/
├── CMakeLists.txt
├── toolchain-mingw.cmake
├── README.md
├── .gitignore
├── include/
│   └── fud_crypter/
│       ├── config.hpp
│       ├── platform.hpp
│       ├── builder/
│       │   ├── builder.hpp
│       │   └── polymorph.hpp
│       ├── carrier/
│       │   ├── icarrier.hpp
│       │   ├── factory.hpp
│       │   ├── pdf_carrier.hpp
│       │   ├── gif_carrier.hpp
│       │   └── png_carrier.hpp
│       ├── crypto/
│       │   ├── icrypto.hpp
│       │   ├── factory.hpp
│       │   └── xor_crypto.hpp
│       ├── loader/
│       │   └── loader.hpp
│       ├── memory/
│       │   └── executable_memory.hpp
│       ├── logging/
│       │   └── logger.hpp
│       ├── api_resolver.hpp
│       ├── syscall.hpp
│       ├── sleep_obf.hpp
│       └── hollowing.hpp
├── src/
│   ├── main.cpp
│   ├── builder/
│   │   ├── builder.cpp
│   │   ├── pdf_builder.cpp
│   │   ├── gif_builder.cpp
│   │   ├── png_builder.cpp
│   │   └── polymorph.cpp
│   ├── carrier/
│   │   ├── factory.cpp
│   │   ├── pdf_carrier.cpp
│   │   ├── gif_carrier.cpp
│   │   └── png_carrier.cpp
│   ├── crypto/
│   │   ├── factory.cpp
│   │   └── xor_crypto.cpp
│   ├── loader/
│   │   └── loader.cpp
│   ├── memory/
│   │   └── executable_memory.cpp
│   ├── platform/
│   │   ├── linux.cpp
│   │   └── windows.cpp
│   ├── api_resolver.cpp
│   ├── syscall.cpp
│   ├── syscall.asm
│   ├── sleep_obf.cpp
│   └── hollowing.cpp
├── docs/
│   └── ARCHITECTURE.md
└── examples/
    └── basic_usage.cpp
```

---

## License

NO LICENSE

---

## Author

**B3r0**  
GitHub: [@b33r00](https://github.com/b33r00)
