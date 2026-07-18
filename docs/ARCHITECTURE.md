# docs/ARCHITECTURE.md

# Carrier Engine Architecture

## Overview

Carrier Engine is a modular framework for extracting, decrypting, and executing 
payloads embedded in various file formats (carriers).

## Architecture Diagram

\`\`\`
CLI (main.cpp)
 │
 ▼
Loader (loader.hpp/cpp)
 │
 ├─ ICarrier ◄── CarrierFactory
 │   ├─ GifCarrier
 │   ├─ PngCarrier
 │   └─ PdfCarrier
 │
 ├─ ICrypto ◄── CryptoFactory
 │   ├─ XorCrypto
 │   └─ AesCrypto (planned)
 │
 ├─ ExecutableMemory
 │   └─ Platform (windows.cpp/linux.cpp)
 │
 └─ Logger (Dependency Injected)
\`\`\`

## Component Responsibilities

### Loader
Orchestrates the extraction-decryption-execution pipeline.
- Coordinates between components
- Handles the full lifecycle
- Single point of error handling

### ICarrier
Interface for extracting payloads from file formats.
- PDF: Object stream extraction
- PNG: iTXt chunk extraction  
- GIF: Application Extension extraction

### ICrypto
Interface for cryptographic operations.
- XOR: Simple symmetric obfuscation
- AES-256-GCM: Authenticated encryption (planned)

### Platform
Abstracts OS-specific APIs behind a uniform interface.
- Memory allocation (VirtualAlloc/mmap)
- Thread management (CreateThread/pthread)
- File I/O

### Logger
Observable logging with dependency injection for testing.
- Multiple log levels
- Custom output streams
- No global state

## Data Flow

1. CLI parses arguments → LoaderConfig
2. Loader creates Carrier (via Factory)
3. Carrier reads file → extracts encrypted payload
4. Loader creates Crypto (via Factory)
5. Crypto decrypts payload → plain shellcode
6. Loader allocates ExecutableMemory
7. Shellcode written to memory → protection changed to RX
8. New thread spawned → shellcode executes
