# docs/COMPONENTS.md

# Adding New Components

## Adding a New Carrier

1. Create `include/carrier_engine/carrier/your_carrier.hpp`
2. Implement `ICarrier` interface
3. Add to `src/carrier/factory.cpp`
4. Register in `detect_file_type()`

Example: [PDF Carrier Implementation](examples/custom_carrier.cpp)

## Adding a New Crypto Algorithm

1. Create `include/carrier_engine/crypto/your_crypto.hpp`
2. Implement `ICrypto` interface
3. Add to `crypto/factory.hpp` enum
4. Create factory function

## Adding Platform Support

1. Create `src/platform/your_platform.cpp`
2. Implement all functions from `platform.hpp`
3. Add to CMakeLists.txt platform detection
