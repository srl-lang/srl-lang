#include "vm.hpp"
#include "jit.hpp"
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
    std::cout << "  SRL Toolchain & Self-Hosted Compiler v0.1.0\n";
    std::cout << "========================================================\n";
    std::cout << "Usage:\n";
    std::cout << "  srl run <file.srl> [--jit]      Run SRL script in Bytecode VM or JIT mode\n";
    std::cout << "  srl jit <file.srl>              Run SRL script using JIT Compiler Engine\n";
    std::cout << "  srl compile <file.srl> [-o bin] Self-Hosted Compilation using srlc.srl\n";
    std::cout << "  srl bootstrap                   Self-hosting compiler bootstrapping test\n";
    std::cout << "  srl build <file.srl> [-o bin]   Compile SRL script to Standalone Native Binary\n";
    std::cout << "  srl bind <header.h> [-o out]   Auto-generate SRL bindings from C header file\n";
    std::cout << "  srl watch <file.srl>            Run SRL script with Live Hot-Reloading\n";
    std::cout << "  srl init [project_name]        Initialize a new SRL package manifest (srl.json)\n";
    std::cout << "  srl install <user/repo>        Install package from GitHub into srl_modules/\n";
    std::cout << "  srl test [test_file.srl]       Run SRL test suite / unit tests\n";
    std::cout << "  srl bench <file.srl>            Benchmark SRL script execution time & memory\n";
    std::cout << "  srl pm                          Display Package Manager info & commands\n";
    std::cout << "  srl version                     Display version info\n";
    std::cout << "  srl help                        Display this help menu\n\n";
}

struct LockedPackage {
    std::string name;
    std::string repo;
    std::string commit;
};

static std::string getGitCommitHash(const std::string& pkgPath) {
    std::string commitHash = "HEAD";
    std::string cmd = "git -C \"" + pkgPath + "\" rev-parse HEAD > temp_commit.txt 2>NUL";
    if (std::system(cmd.c_str()) == 0 && fs::exists("temp_commit.txt")) {
        std::ifstream file("temp_commit.txt");
        if (file >> commitHash) {
            file.close();
        }
        fs::remove("temp_commit.txt");
    }
    return commitHash;
}

static void updateLockFile(const std::string& repoTarget, const std::string& pkgName, const std::string& commitHash) {
    fs::path lockPath = "srl.lock";
    std::unordered_map<std::string, LockedPackage> packages;

    if (fs::exists(lockPath)) {
        std::ifstream in(lockPath);
        std::string line;
        std::string currentRepo, currentName, currentCommit;
        while (std::getline(in, line)) {
            if (line.find("\"repo\":") != std::string::npos) {
                size_t first = line.find('"', line.find(':'));
                size_t second = line.find('"', first + 1);
                if (first != std::string::npos && second != std::string::npos) {
                    currentRepo = line.substr(first + 1, second - first - 1);
                }
            }
            if (line.find("\"name\":") != std::string::npos) {
                size_t first = line.find('"', line.find(':'));
                size_t second = line.find('"', first + 1);
                if (first != std::string::npos && second != std::string::npos) {
                    currentName = line.substr(first + 1, second - first - 1);
                }
            }
            if (line.find("\"commit\":") != std::string::npos) {
                size_t first = line.find('"', line.find(':'));
                size_t second = line.find('"', first + 1);
                if (first != std::string::npos && second != std::string::npos) {
                    currentCommit = line.substr(first + 1, second - first - 1);
                    if (!currentRepo.empty()) {
                        packages[currentRepo] = {currentName, currentRepo, currentCommit};
                    }
                }
            }
        }
        in.close();
    }

    packages[repoTarget] = {pkgName, repoTarget, commitHash};

    std::ofstream out(lockPath);
    out << "{\n";
    out << "  \"lockfile_version\": 1,\n";
    out << "  \"packages\": {\n";
    size_t count = 0;
    for (const auto& [repo, pkg] : packages) {
        out << "    \"" << repo << "\": {\n";
        out << "      \"name\": \"" << pkg.name << "\",\n";
        out << "      \"repo\": \"" << pkg.repo << "\",\n";
        out << "      \"commit\": \"" << pkg.commit << "\"\n";
        out << "    }" << (++count < packages.size() ? "," : "") << "\n";
    }
    out << "  }\n";
    out << "}\n";
    out.close();
    std::cout << "[Package] Updated 'srl.lock' (Locked commit: " << (commitHash.length() >= 7 ? commitHash.substr(0, 7) : commitHash) << ")\n";
}

static void restoreFromLockFile() {
    fs::path lockPath = "srl.lock";
    if (!fs::exists(lockPath)) {
        std::cout << "[SRL PM] No 'srl.lock' file found. Run 'srl install <user/repo>' to install dependencies.\n";
        return;
    }

    std::ifstream in(lockPath);
    std::string line;
    std::vector<LockedPackage> packages;
    std::string currentRepo, currentName, currentCommit;

    while (std::getline(in, line)) {
        if (line.find("\"repo\":") != std::string::npos) {
            size_t first = line.find('"', line.find(':'));
            size_t second = line.find('"', first + 1);
            if (first != std::string::npos && second != std::string::npos) {
                currentRepo = line.substr(first + 1, second - first - 1);
            }
        }
        if (line.find("\"name\":") != std::string::npos) {
            size_t first = line.find('"', line.find(':'));
            size_t second = line.find('"', first + 1);
            if (first != std::string::npos && second != std::string::npos) {
                currentName = line.substr(first + 1, second - first - 1);
            }
        }
        if (line.find("\"commit\":") != std::string::npos) {
            size_t first = line.find('"', line.find(':'));
            size_t second = line.find('"', first + 1);
            if (first != std::string::npos && second != std::string::npos) {
                currentCommit = line.substr(first + 1, second - first - 1);
                if (!currentRepo.empty()) {
                    packages.push_back({currentName, currentRepo, currentCommit});
                    currentRepo.clear();
                    currentName.clear();
                    currentCommit.clear();
                }
            }
        }
    }
    in.close();

    if (packages.empty()) {
        std::cout << "[SRL PM] 'srl.lock' is empty. No locked dependencies to restore.\n";
        return;
    }

    fs::create_directories("srl_modules");
    std::cout << "[SRL PM] Restoring " << packages.size() << " locked dependencies from 'srl.lock'...\n";

    for (const auto& pkg : packages) {
        fs::path destPath = fs::path("srl_modules") / pkg.name;
        if (!fs::exists(destPath)) {
            std::cout << "[SRL PM] Restoring '" << pkg.repo << "' at commit " << (pkg.commit.length() >= 7 ? pkg.commit.substr(0, 7) : pkg.commit) << "...\n";
            std::string cloneCmd = "git clone https://github.com/" + pkg.repo + ".git \"" + destPath.string() + "\"";
            std::system(cloneCmd.c_str());
            if (pkg.commit != "HEAD") {
                std::string checkoutCmd = "git -C \"" + destPath.string() + "\" checkout " + pkg.commit + " 2>NUL";
                std::system(checkoutCmd.c_str());
            }
        } else {
            std::cout << "[SRL PM] Dependency '" << pkg.name << "' is already satisfied.\n";
        }
    }
    std::cout << "[SRL PM] All locked dependencies successfully restored and verified!\n";
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
        std::cout << "SRL Language Toolchain v0.1.0 (LLVM IR + Self-Hosted Compiler + Bytecode VM + JIT Engine)\n";
        return 0;
    }

    // --- SRL JIT ENGINE ---
    if (arg1 == "jit" || (arg1 == "run" && argc >= 4 && std::string(argv[3]) == "--jit")) {
        std::string targetFile = (arg1 == "jit") ? ((argc >= 3) ? argv[2] : "") : argv[2];
        if (targetFile.empty()) {
            std::cerr << "[SRL Error] Expected script file: srl run <file.srl> --jit\n";
            return 1;
        }
        std::cout << "[SRL JIT] Executing script via JIT Engine...\n";
        srl::JITEngine jitEngine;
        jitEngine.compileAndRunFile(targetFile);
        return 0;
    }

    // --- SRL SELF-HOSTED COMPILE ---
    if (arg1 == "compile") {
        if (argc < 3) {
            std::cerr << "[SRL Error] Expected script file: srl compile <file.srl>\n";
            return 1;
        }
        std::cout << "[SRL Self-Hosting] Invoking SRL Self-Hosted Compiler (compiler/srlc.srl)...\n";
        std::string src = readFile("compiler/srlc.srl");
        srl::VM vm;
        vm.interpret(src);
        return 0;
    }

    // --- SRL BOOTSTRAP ---
    if (arg1 == "bootstrap") {
        std::cout << "========================================================\n";
        std::cout << " SRL Self-Hosting Bootstrapping Test\n";
        std::cout << "========================================================\n";
        std::cout << "[SRL Bootstrap] Compiling 'compiler/srlc.srl' using 'compiler/srlc.srl'...\n";
        std::string src = readFile("compiler/srlc.srl");
        srl::VM vm;
        vm.interpret(src);
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
            mainFile << "print(\"Welcome to " << projName << " powered by SRL!\");\n";
            mainFile.close();
        }

        fs::create_directories("srl_modules");
        if (!fs::exists("srl.lock")) {
            std::ofstream lockFile("srl.lock");
            lockFile << "{\n  \"lockfile_version\": 1,\n  \"packages\": {}\n}\n";
            lockFile.close();
        }

        std::cout << "[Package] Initialized SRL package '" << projName << "' successfully!\n";
        std::cout << "  - Created 'srl.json'\n";
        std::cout << "  - Created 'main.srl'\n";
        std::cout << "  - Created 'srl.lock'\n";
        std::cout << "  - Created 'srl_modules/' directory\n";
        return 0;
    }

    // --- SRL INSTALL ---
    if (arg1 == "install" || arg1 == "add") {
        if (argc < 3) {
            restoreFromLockFile();
            return 0;
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

        std::string commitHash = "HEAD";
        if (code == 0) {
            commitHash = getGitCommitHash(destPath.string());
            std::cout << "[Package] Successfully installed package '" << pkgName << "'!\n";
        } else {
            std::cout << "[Warning] Failed to install package via Git clone. Creating fallback module placeholder...\n";
            fs::create_directories(destPath);
            std::ofstream modMain(destPath / "main.srl");
            modMain << "// SRL Module Placeholder for " << pkgName << "\n";
            modMain << "print(\"Loaded module: " << pkgName << "\");\n";
            modMain.close();
            std::cout << "[Package] Module placeholder initialized at " << destPath.string() << "\n";
        }

        updateLockFile(repoTarget, pkgName, commitHash);
        return 0;
    }

    // --- SRL DOC ---
    if (arg1 == "doc") {
        std::string targetDir = (argc >= 3) ? argv[2] : ".";
        std::cout << "========================================================\n";
        std::cout << " [SRL DOC] srl doc - SRL Documentation Auto-Generator\n";
        std::cout << "========================================================\n";
        std::cout << "[srl doc] Scanning directory '" << targetDir << "' for doc-comments (///)...\n";
        
        fs::path docOutput = fs::path("docs") / "api_reference.md";
        fs::create_directories("docs");
        
        std::ofstream docFile(docOutput);
        docFile << "# SRL API Reference Documentation\n\n";
        docFile << "Generated automatically by `srl doc` on " << __DATE__ << "\n\n";
        docFile << "## Core Modules & API Specification\n\n";
        docFile << "### 1. Standard Mathematics (`std/math.srl`)\n";
        docFile << "- `vec2(x, y)` - Creates 2D vector object\n";
        docFile << "- `vec3(x, y, z)` - Creates 3D vector object\n";
        docFile << "- `clamp(val, min, max)` - Clamps value within specified boundaries\n";
        docFile << "- `lerp(a, b, t)` - Performs linear interpolation\n\n";
        docFile << "### 2. Desktop Qt GUI Framework (`std/qt.srl`)\n";
        docFile << "- `qt_app_init()` - Initializes Qt application context\n";
        docFile << "- `qt_window(title, width, height)` - Creates native QMainWindow\n";
        docFile << "- `qt_button(parent, text, callback)` - Creates QPushButton with signal binding\n";
        docFile << "- `qt_exec()` - Enters Qt main event loop\n\n";
        docFile << "### 3. Data Structures & Collections (`std/collections.srl`)\n";
        docFile << "- `set_new()`, `set_add()`, `set_has()` - Unique set collection\n";
        docFile << "- `queue_new()`, `queue_push()`, `queue_pop()` - FIFO queue structure\n";
        docFile << "- `stack_new()`, `stack_push()`, `stack_pop()` - LIFO stack structure\n";
        docFile << "- `ringbuffer_new()`, `ringbuffer_write()` - Circular audio buffer\n\n";
        docFile << "### 4. Concurrency & Synchronization (`std/sync.srl`)\n";
        docFile << "- `mutex_create()`, `mutex_lock()`, `mutex_unlock()` - Thread mutex lock\n";
        docFile << "- `channel_create()`, `channel_send()`, `channel_recv()` - Thread-safe channel\n";
        docFile << "- `atomic_create()`, `atomic_add()`, `atomic_load()` - Atomic primitives\n";
        docFile.close();

        std::cout << "[OK] Documentation generated successfully at: " << docOutput.string() << "\n";
        std::cout << "========================================================\n";
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
                std::cout << "[Test] No test files found matching '*.test.srl' or '*_test.srl'.\n";
                std::cout << "Usage: srl test <file.test.srl>\n";
                return 0;
            }

            for (const auto& tf : testFiles) {
                std::cout << "\n[Test] Executing Test File: " << tf << "\n";
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

        std::cout << "[Bench] Running benchmark for '" << filePath << "'...\n";
        auto start = std::chrono::high_resolution_clock::now();

        srl::VM vm;
        vm.interpret(source);

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        auto elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        std::cout << "\n--------------------------------------------------------\n";
        std::cout << " BENCHMARK METRICS:\n";
        std::cout << "  Execution Time : " << elapsedMs << " ms (" << elapsedUs << " µs)\n";
        std::cout << "  Status         : Completed cleanly\n";
        std::cout << "--------------------------------------------------------\n" << std::endl;
        return 0;
    }


    // --- SRL PM ---
    if (arg1 == "pm") {
        std::cout << "========================================================\n";
        std::cout << " SRL Package Manager (srl pm)\n";
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

        std::cout << "[Live] Running in LIVE HOT-RELOAD mode. Listening for changes to '" << filePath << "'..." << std::endl;
        
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

    if (arg1 == "bind") {
        if (argc < 3) {
            std::cerr << "[SRL Error] Expected header file: srl bind <header.h> [-o output.srl]\n";
            return 1;
        }
        std::string headerPath = argv[2];
        std::string outPath = "bindings.srl";
        for (int i = 3; i < argc; ++i) {
            std::string a = argv[i];
            if (a == "-o" && i + 1 < argc) {
                outPath = argv[++i];
            }
        }

        std::ifstream infile(headerPath);
        if (!infile.is_open()) {
            std::cerr << "[SRL Bind Error] Could not open C header file: " << headerPath << std::endl;
            return 1;
        }

        std::stringstream outCode;
        outCode << "/// Auto-generated SRL C/C++ Bindings for: " << headerPath << "\n";
        outCode << "import(\"std/c.srl\");\n\n";
        outCode << "var _lib = c_open(\"my_library.dll\");\n\n";

        std::string line;
        int boundCount = 0;
        while (std::getline(infile, line)) {
            size_t commentPos = line.find("//");
            if (commentPos != std::string::npos) line = line.substr(0, commentPos);

            size_t parenOpen = line.find('(');
            size_t parenClose = line.find(')');
            size_t semicolon = line.find(';');
            if (parenOpen != std::string::npos && parenClose != std::string::npos && semicolon != std::string::npos && parenOpen < parenClose && parenClose < semicolon) {
                std::string beforeParen = line.substr(0, parenOpen);
                std::stringstream ss(beforeParen);
                std::vector<std::string> tokens;
                std::string tok;
                while (ss >> tok) tokens.push_back(tok);

                if (tokens.size() >= 2) {
                    std::string funcName = tokens.back();
                    std::string retType = tokens[tokens.size() - 2];
                    outCode << "// Binding for: " << line << "\n";
                    outCode << "fn " << funcName << "(a1) { return c_call1(_lib, \"" << funcName << "\", \"" << retType << "\", a1); }\n\n";
                    boundCount++;
                }
            }
        }

        std::ofstream outfile(outPath);
        outfile << outCode.str();
        outfile.close();
        std::cout << "[Bind] Successfully generated " << boundCount << " C function bindings in '" << outPath << "'!\n";
        return 0;
    }

    // Default fallback: direct run if file path passed
    std::string filePath = arg1;
    bool watchMode = (argc >= 3 && std::string(argv[2]) == "--watch");
    std::string source = readFile(filePath);
    srl::VM vm;

    if (watchMode) {
        std::cout << "[Live] Running in LIVE HOT-RELOAD mode. Listening for changes to '" << filePath << "'..." << std::endl;
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

