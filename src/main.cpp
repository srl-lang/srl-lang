#include "vm.hpp"
#include "watcher.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

static std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[SRL Error] Could not open file: " << path << std::endl;
        exit(1);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

static void printUsage() {
    std::cout << "========================================================\n";
    std::cout << "  SRL Toolchain (Serial Run Language) v0.1.0\n";
    std::cout << "========================================================\n";
    std::cout << "Usage:\n";
    std::cout << "  srl run <file.srl>              Run SRL script in Bytecode VM\n";
    std::cout << "  srl build <file.srl> [-o binary] Compile SRL script to Standalone Native Binary\n";
    std::cout << "  srl watch <file.srl>            Run SRL script with Live Hot-Reloading\n";
    std::cout << "  srl version                     Display version info\n";
    std::cout << "  srl help                        Display this help menu\n\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 0;
    }

    std::string arg1 = argv[1];

    if (arg1 == "help" || arg1 == "-h" || arg1 == "--help") {
        printUsage();
        return 0;
    }

    if (arg1 == "version" || arg1 == "-v" || arg1 == "--version") {
        std::cout << "SRL Language Toolchain v0.1.0 (LLVM IR + C++ Bytecode VM)\n";
        return 0;
    }

    // Command dispatch: run, build, watch
    if (arg1 == "run") {
        if (argc < 3) {
            std::cerr << "[SRL Error] Expected script file: srl run <file.srl>\n";
            return 1;
        }
        std::string filePath = argv[2];
        std::string source = readFile(filePath);
        srl::VM vm;
        vm.interpret(source);
        return 0;
    }

    if (arg1 == "watch") {
        if (argc < 3) {
            std::cerr << "[SRL Error] Expected script file: srl watch <file.srl>\n";
            return 1;
        }
        std::string filePath = argv[2];
        std::string source = readFile(filePath);
        srl::VM vm;

        std::cout << "🔥 Running in LIVE HOT-RELOAD mode. Listening for changes to '" << filePath << "'..." << std::endl;
        
        srl::FileWatcher watcher(filePath, [&vm](const std::string& newSource) {
            vm.interpret(newSource);
        });
        watcher.start();
        vm.interpret(source);
        watcher.stop();
        return 0;
    }

    if (arg1 == "build") {
        if (argc < 3) {
            std::cerr << "[SRL Error] Expected script file: srl build <file.srl> [-o output.exe]\n";
            return 1;
        }
        std::string filePath = argv[2];
        std::string outOption = "";
        for (int i = 3; i < argc; ++i) {
            std::string a = argv[i];
            if (a == "-o" && i + 1 < argc) {
                outOption = " -o " + std::string(argv[++i]);
            }
        }

        fs::path exeDir = fs::canonical(fs::path(argv[0])).parent_path();
        fs::path srlcPath = exeDir / "srlc.exe";
        if (!fs::exists(srlcPath)) {
            srlcPath = exeDir / ".." / "srlc" / "build" / "Release" / "srlc.exe";
        }
        if (!fs::exists(srlcPath)) {
            srlcPath = "srlc";
        }

        std::string buildCmd = "\"" + srlcPath.string() + "\" \"" + filePath + "\"" + outOption;
        std::cout << "[SRL Build] Executing: " << buildCmd << std::endl;
        return std::system(buildCmd.c_str());
    }

    // Default fallback: direct run if file path passed
    std::string filePath = arg1;
    bool watchMode = (argc >= 3 && std::string(argv[2]) == "--watch");
    std::string source = readFile(filePath);
    srl::VM vm;

    if (watchMode) {
        std::cout << "🔥 Running in LIVE HOT-RELOAD mode. Listening for changes to '" << filePath << "'..." << std::endl;
        srl::FileWatcher watcher(filePath, [&vm](const std::string& newSource) {
            vm.interpret(newSource);
        });
        watcher.start();
        vm.interpret(source);
        watcher.stop();
    } else {
        vm.interpret(source);
    }

    return 0;
}
