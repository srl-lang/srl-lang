#include "vm.hpp"
#include "watcher.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << path << std::endl;
        exit(1);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char* argv[]) {
    std::cout << "===============================================" << std::endl;
    std::cout << "  Serial Run Language (.srl) v0.10 Engine      " << std::endl;
    std::cout << "  C++ Bytecode VM & Live Hot-Reload System     " << std::endl;
    std::cout << "===============================================\n" << std::endl;

    if (argc < 2) {
        std::cout << "Usage:\n";
        std::cout << "  srl <file.srl>          Run SRL script\n";
        std::cout << "  srl <file.srl> --watch  Run SRL script with live hot reload\n";
        return 0;
    }

    std::string filePath = argv[1];
    bool watchMode = false;
    if (argc >= 3 && std::string(argv[2]) == "--watch") {
        watchMode = true;
    }

    std::string source = readFile(filePath);
    srl::VM vm;

    if (watchMode) {
        std::cout << "🔥 Running in LIVE HOT-RELOAD mode. Listening for changes to '" << filePath << "'..." << std::endl;
        
        // Start file watcher thread FIRST so it monitors during script execution
        srl::FileWatcher watcher(filePath, [&vm](const std::string& newSource) {
            vm.interpret(newSource);
        });
        watcher.start();

        // Initial run
        vm.interpret(source);

        watcher.stop();
    } else {
        vm.interpret(source);
    }

    return 0;
}
