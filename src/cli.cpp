#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "cli.hpp"
#include "installer.hpp"
#include "vm.hpp"

#include "lexer.hpp"
#include "parser.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

namespace srl {
namespace cli {

void Term::enableAnsi() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
}

std::string Term::bold(const std::string& text) { return "\033[1m" + text + "\033[0m"; }
std::string Term::green(const std::string& text) { return "\033[32m" + text + "\033[0m"; }
std::string Term::red(const std::string& text) { return "\033[31m" + text + "\033[0m"; }
std::string Term::yellow(const std::string& text) { return "\033[33m" + text + "\033[0m"; }
std::string Term::cyan(const std::string& text) { return "\033[36m" + text + "\033[0m"; }
std::string Term::gray(const std::string& text) { return "\033[90m" + text + "\033[0m"; }

void Term::status(const std::string& header, const std::string& message, bool isSuccess) {
    enableAnsi();
    if (isSuccess) {
        std::cout << "\033[1;32m" << header << "\033[0m " << message << std::endl;
    } else {
        std::cout << "\033[1;31m" << header << "\033[0m " << message << std::endl;
    }
}

void Term::error(const std::string& header, const std::string& message) {
    enableAnsi();
    std::cerr << "\033[1;31m" << header << "\033[0m " << message << std::endl;
}

void Term::warn(const std::string& header, const std::string& message) {
    enableAnsi();
    std::cout << "\033[1;33m" << header << "\033[0m " << message << std::endl;
}

void printVersion() {
    Term::enableAnsi();
    std::cout << Term::bold("SRL Language Compiler Toolchain") << " v0.3.2\n";
    std::cout << "Target: x86_64-pc-windows-msvc (Bytecode VM & AOT JIT Engine)\n";
}

void printHelp() {
    Term::enableAnsi();
    std::cout << Term::bold("SRL Compiler & Package Toolchain Usage:") << "\n";
    std::cout << "  srl <subcommand> [options]\n\n";
    std::cout << Term::bold("Project Commands:") << "\n";
    std::cout << "  " << Term::cyan("new <name> [--lib|--bin]") << "    Create a new SRL project directory\n";
    std::cout << "  " << Term::cyan("init") << "                     Initialize a new srl.json package manifest\n";
    std::cout << "  " << Term::cyan("build [--release]") << "          Compile project into AOT standalone executable\n";
    std::cout << "  " << Term::cyan("run [file.srl]") << "            Execute SRL script or current project\n";
    std::cout << "  " << Term::cyan("check [file.srl]") << "          Perform fast static analysis and syntax validation\n";
    std::cout << "  " << Term::cyan("test [file.srl]") << "           Run project test suite (*.test.srl)\n";
    std::cout << "  " << Term::cyan("fmt [file.srl]") << "            Auto-format SRL source files\n";
    std::cout << "  " << Term::cyan("clean") << "                    Clean build artifacts (build/, bin/)\n";
    std::cout << "  " << Term::cyan("doc") << "                      Generate HTML documentation from comments\n";
    std::cout << "  " << Term::cyan("setup") << "                    Run native toolchain installer & environment setup\n";
    std::cout << "  " << Term::cyan("installer <file.srl>") << "      Build standalone GUI setup installer for project\n\n";


    std::cout << Term::bold("Package Manager Commands:") << "\n";
    std::cout << "  " << Term::cyan("add <user/repo>") << "           Add package dependency to srl.json\n";
    std::cout << "  " << Term::cyan("remove <name>") << "             Remove package dependency from srl.json\n\n";
    std::cout << Term::bold("General Options:") << "\n";
    std::cout << "  " << Term::cyan("version") << "                  Display compiler version information\n";
    std::cout << "  " << Term::cyan("help") << "                     Display this help menu\n\n";
}

int handleNew(int argc, char* argv[]) {
    if (argc < 3) {
        Term::error("Error:", "Expected project name: srl new <project_name>");
        return 1;
    }
    std::string projName = argv[2];
    bool isLib = (argc >= 4 && std::string(argv[3]) == "--lib");

    fs::path projDir = projName;
    if (fs::exists(projDir)) {
        Term::error("Error:", "Directory '" + projName + "' already exists.");
        return 1;
    }

    fs::create_directories(projDir / "src");

    // Write srl.json
    std::ofstream manifest(projDir / "srl.json");
    manifest << "{\n";
    manifest << "  \"name\": \"" << projName << "\",\n";
    manifest << "  \"version\": \"0.1.0\",\n";
    manifest << "  \"type\": \"" << (isLib ? "library" : "binary") << "\",\n";
    manifest << "  \"dependencies\": {}\n";
    manifest << "}\n";
    manifest.close();

    // Write src/main.srl or src/lib.srl
    if (isLib) {
        std::ofstream libFile(projDir / "src" / "lib.srl");
        libFile << "// " << projName << " Library\n\n";
        libFile << "fn add(a: number, b: number): number {\n";
        libFile << "    return a + b;\n";
        libFile << "}\n";
        libFile.close();
    } else {
        std::ofstream mainFile(projDir / "src" / "main.srl");
        mainFile << "// " << projName << " Main Entry Point\n\n";
        mainFile << "fn main() {\n";
        mainFile << "    print(\"Hello from " << projName << "!\");\n";
        mainFile << "}\n\n";
        mainFile << "main();\n";
        mainFile.close();
    }

    // Write .gitignore
    std::ofstream gitignore(projDir / ".gitignore");
    gitignore << "build/\nbin/\nsrl_modules/\nsrl.lock\n";
    gitignore.close();

    Term::status("Created", (isLib ? "library " : "binary ") + projName + " project successfully.");
    return 0;
}

int handleInit(int argc, char* argv[]) {
    fs::path manifestPath = "srl.json";
    if (fs::exists(manifestPath)) {
        Term::warn("Warning:", "srl.json manifest already exists in current directory.");
        return 0;
    }
    std::string currentDirName = fs::current_path().filename().string();
    std::ofstream manifest(manifestPath);
    manifest << "{\n";
    manifest << "  \"name\": \"" << currentDirName << "\",\n";
    manifest << "  \"version\": \"0.1.0\",\n";
    manifest << "  \"type\": \"binary\",\n";
    manifest << "  \"dependencies\": {}\n";
    manifest << "}\n";
    manifest.close();

    Term::status("Initialized", "srl.json in " + currentDirName);
    return 0;
}

int handleCheck(int argc, char* argv[]) {
    std::string targetFile = (argc >= 3) ? argv[2] : "src/main.srl";
    if (!fs::exists(targetFile)) {
        if (fs::exists("main.srl")) targetFile = "main.srl";
        else {
            Term::error("Error:", "Target script file '" + targetFile + "' not found.");
            return 1;
        }
    }

    Term::status("Checking", targetFile);
    auto start = std::chrono::high_resolution_clock::now();

    std::ifstream file(targetFile);
    if (!file.is_open()) {
        Term::error("Error:", "Could not open file: " + targetFile);
        return 1;
    }
    std::stringstream ss;
    ss << file.rdbuf();
    std::string source = ss.str();

    try {
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(tokens);
        auto stmts = parser.parse();

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        Term::status("Finished", "check in " + std::to_string(duration) + "ms (0 errors)");
    } catch (const std::exception& e) {
        Term::error("Check Failed:", e.what());
        return 1;
    }
    return 0;
}

int handleClean(int argc, char* argv[]) {
    size_t removed = 0;
    if (fs::exists("build")) { fs::remove_all("build"); removed++; }
    if (fs::exists("bin")) { fs::remove_all("bin"); removed++; }
    Term::status("Cleaned", std::to_string(removed) + " build directory artifact(s)");
    return 0;
}

int handleFmt(int argc, char* argv[]) {
    std::string targetFile = (argc >= 3) ? argv[2] : "src/main.srl";
    if (!fs::exists(targetFile)) {
        Term::error("Error:", "Target file '" + targetFile + "' not found.");
        return 1;
    }

    std::ifstream file(targetFile);
    std::string line;
    std::vector<std::string> formattedLines;
    int indentLevel = 0;

    while (std::getline(file, line)) {
        // Trim leading/trailing whitespace
        size_t first = line.find_first_not_of(" \t");
        if (first == std::string::npos) {
            formattedLines.push_back("");
            continue;
        }
        size_t last = line.find_last_not_of(" \t");
        std::string trimmed = line.substr(first, (last - first + 1));

        if (!trimmed.empty() && trimmed[0] == '}') {
            indentLevel = std::max(0, indentLevel - 1);
        }

        std::string indentedLine = std::string(indentLevel * 4, ' ') + trimmed;
        formattedLines.push_back(indentedLine);

        if (!trimmed.empty() && trimmed.back() == '{') {
            indentLevel++;
        }
    }
    file.close();

    std::ofstream out(targetFile);
    for (size_t i = 0; i < formattedLines.size(); ++i) {
        out << formattedLines[i] << "\n";
    }
    out.close();

    Term::status("Formatted", targetFile);
    return 0;
}

int handleSetup(int argc, char* argv[]) {
    return srl::installer::runInstallerCli(argc, argv);
}

int handleInstallerCmd(int argc, char* argv[]) {
    return srl::installer::handleInstallerCli(argc, argv);
}

} // namespace cli
} // namespace srl


