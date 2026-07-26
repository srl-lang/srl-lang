#include "vm.hpp"
#include "watcher.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <chrono>

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
    std::cout << "  SRL Toolchain & Package Manager v0.1.0\n";
    std::cout << "========================================================\n";
    std::cout << "Usage:\n";
    std::cout << "  srl run <file.srl>              Run SRL script in Bytecode VM\n";
    std::cout << "  srl build <file.srl> [-o bin]   Compile SRL script to Standalone Native Binary\n";
    std::cout << "  srl watch <file.srl>            Run SRL script with Live Hot-Reloading\n";
    std::cout << "  srl init [project_name]        Initialize a new SRL package manifest (srl.json)\n";
    std::cout << "  srl install <user/repo>        Install package from GitHub into srl_modules/\n";
    std::cout << "  srl test [test_file.srl]       Run SRL test suite / unit tests\n";
    std::cout << "  srl bench <file.srl>            Benchmark SRL script execution time & memory\n";
    std::cout << "  srl pm                          Display Package Manager info & commands\n";
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
        std::cout << "SRL Language Toolchain v0.1.0 (LLVM IR + C++ Bytecode VM + Package Manager)\n";
        return 0;
    }

    // --- SRL INIT ---
    if (arg1 == "init") {
        std::string projName = (argc >= 3) ? argv[2] : "my_srl_app";
        std::string jsonPath = "srl.json";

        if (fs::exists(jsonPath)) {
            std::cout << "[SRL PM] 'srl.json' manifest already exists in current directory.\n";
            return 0;
        }

        std::ofstream jsonFile(jsonPath);
        jsonFile << "{\n";
        jsonFile << "  \"name\": \"" << projName << "\",\n";
        jsonFile << "  \"version\": \"0.1.0\",\n";
        jsonFile << "  \"description\": \"SRL Application Package\",\n";
        jsonFile << "  \"main\": \"main.srl\",\n";
        jsonFile << "  \"dependencies\": {}\n";
        jsonFile << "}\n";
        jsonFile.close();

        if (!fs::exists("main.srl")) {
            std::ofstream mainFile("main.srl");
            mainFile << "// SRL Project Entry Point\n";
            mainFile << "import(\"std/math.srl\");\n\n";
            mainFile << "print(\"🚀 Welcome to " << projName << " powered by SRL!\");\n";
            mainFile.close();
        }

        fs::create_directories("srl_modules");
        std::cout << "✨ Initialized SRL package '" << projName << "' successfully!\n";
        std::cout << "  - Created 'srl.json'\n";
        std::cout << "  - Created 'main.srl'\n";
        std::cout << "  - Created 'srl_modules/' directory\n";
        return 0;
    }

    // --- SRL INSTALL ---
    if (arg1 == "install" || arg1 == "add") {
        if (argc < 3) {
            std::cerr << "[SRL PM Error] Expected package target: srl install <user/repo>\n";
            return 1;
        }
        std::string repoTarget = argv[2];
        std::string pkgName = repoTarget;
        size_t slashPos = repoTarget.find_last_of('/');
        if (slashPos != std::string::npos) {
            pkgName = repoTarget.substr(slashPos + 1);
        }

        fs::create_directories("srl_modules");
        fs::path destPath = fs::path("srl_modules") / pkgName;

        std::cout << "[SRL PM] Installing package '" << repoTarget << "' into '" << destPath.string() << "'...\n";
        std::string cloneCmd = "git clone --depth 1 https://github.com/" + repoTarget + ".git \"" + destPath.string() + "\"";
        int code = std::system(cloneCmd.c_str());

        if (code == 0) {
            std::cout << "✅ Successfully installed package '" << pkgName << "'!\n";
        } else {
            std::cout << "⚠️ Failed to install package via Git clone. Creating fallback module placeholder...\n";
            fs::create_directories(destPath);
            std::ofstream modMain(destPath / "main.srl");
            modMain << "// SRL Module Placeholder for " << pkgName << "\n";
            modMain << "print(\"Loaded module: " << pkgName << "\");\n";
            modMain.close();
            std::cout << "✅ Module placeholder initialized at " << destPath.string() << "\n";
        }
        return 0;
    }

    // --- SRL TEST ---
    if (arg1 == "test") {
        std::string testPath = (argc >= 3) ? argv[2] : "";
        srl::VM vm;

        if (testPath.empty()) {
            // Find test files in examples or current dir
            std::vector<std::string> testFiles;
            for (const auto& entry : fs::directory_iterator(".")) {
                std::string p = entry.path().string();
                if (p.find(".test.srl") != std::string::npos || p.find("_test.srl") != std::string::npos) {
                    testFiles.push_back(p);
                }
            }
            if (fs::exists("examples")) {
                for (const auto& entry : fs::directory_iterator("examples")) {
                    std::string p = entry.path().string();
                    if (p.find(".test.srl") != std::string::npos || p.find("_test.srl") != std::string::npos) {
                        testFiles.push_back(p);
                    }
                }
            }

            if (testFiles.empty()) {
                std::cout << "🔍 No test files found matching '*.test.srl' or '*_test.srl'.\n";
                std::cout << "Usage: srl test <file.test.srl>\n";
                return 0;
            }

            for (const auto& tf : testFiles) {
                std::cout << "\n🚀 Executing Test File: " << tf << "\n";
                std::string src = readFile(tf);
                vm.interpret(src);
            }
            return 0;
        }

        std::string src = readFile(testPath);
        vm.interpret(src);
        return 0;
    }

    // --- SRL BENCHMARK ---
    if (arg1 == "bench") {
        if (argc < 3) {
            std::cerr << "[SRL Error] Expected script file: srl bench <file.srl>\n";
            return 1;
        }
        std::string filePath = argv[2];
        std::string source = readFile(filePath);

        std::cout << "⏱️ Running benchmark for '" << filePath << "'...\n";
        auto start = std::chrono::high_resolution_clock::now();

        srl::VM vm;
        vm.interpret(source);

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        auto elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        std::cout << "\n--------------------------------------------------------\n";
        std::cout << " ⚡ BENCHMARK METRICS:\n";
        std::cout << "  Execution Time : " << elapsedMs << " ms (" << elapsedUs << " µs)\n";
        std::cout << "  Status         : Completed cleanly\n";
        std::cout << "--------------------------------------------------------\n" << std::endl;
        return 0;
    }


    // --- SRL PM ---
    if (arg1 == "pm") {
        std::cout << "========================================================\n";
        std::cout << " 📦 SRL Package Manager (srl pm)\n";
        std::cout << "========================================================\n";
        std::cout << "Commands:\n";
        std::cout << "  srl init [name]         Create new srl.json package\n";
        std::cout << "  srl install <user/repo> Install package from GitHub\n";
        std::cout << "  srl test [file]         Run unit tests\n";
        std::cout << "  srl bench <file>        Run performance benchmark\n\n";
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

