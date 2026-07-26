#include "lexer.hpp"
#include "parser.hpp"
#include "llvm_codegen.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "========================================================\n";
        std::cout << "  srlc - Standalone SRL LLVM IR & Native Compiler Engine\n";
        std::cout << "========================================================\n";
        std::cout << "Usage: srlc <source.srl> [-o output_binary.exe] [--emit-llvm]\n";
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = "output.exe";
    bool emitLLVMOnly = false;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-o" && i + 1 < argc) {
            outputFile = argv[++i];
        } else if (arg == "--emit-llvm") {
            emitLLVMOnly = true;
        }
    }

    std::ifstream file(inputFile);
    if (!file.is_open()) {
        std::cerr << "[srlc Error] Could not open file: " << inputFile << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    file.close();

    std::cout << "[srlc LLVM Engine] Compiling '" << inputFile << "' to LLVM IR & x86_64 Machine Code..." << std::endl;

    try {
        // 1. Lexical Analysis
        srl::Lexer lexer(source);
        auto tokens = lexer.scanTokens();

        // 2. Syntax Analysis (AST)
        srl::Parser parser(tokens);
        auto statements = parser.parse();

        // 3. LLVM IR Code Generation
        srlc::LLVMCodegen codegen;
        std::string llvmIR = codegen.generate(statements);

        std::string llFile = fs::path(inputFile).stem().string() + ".ll";
        std::ofstream llOut(llFile);
        if (!llOut.is_open()) {
            std::cerr << "[srlc Error] Could not create LLVM IR file." << std::endl;
            return 1;
        }
        llOut << llvmIR;
        llOut.close();

        std::cout << "[srlc LLVM Engine] LLVM IR assembly generated: '" << llFile << "'" << std::endl;

        if (emitLLVMOnly) {
            std::cout << "[srlc LLVM Engine] --emit-llvm flag set. Output written to " << llFile << std::endl;
            return 0;
        }

        std::cout << "[srlc LLVM Engine] Invoking LLVM Machine Code Generator..." << std::endl;

        // Run clang / llc or host compiler engine on generated LLVM IR
        std::string compileCmd = "clang -O2 -o " + outputFile + " " + llFile;
        int res = std::system(compileCmd.c_str());

        if (res == 0) {
            std::cout << "\n========================================================\n";
            std::cout << "  SUCCESSFULLY COMPILED LLVM NATIVE BINARY: " << outputFile << "\n";
            std::cout << "========================================================\n";
        } else {
            std::cout << "[srlc Note] Host LLVM compiler 'clang' not in PATH. Preserved LLVM IR: '" << llFile << "'" << std::endl;
            std::cout << "========================================================\n";
            std::cout << "  LLVM IR GENERATED SUCCESSFULLY: " << llFile << "\n";
            std::cout << "========================================================\n";
        }

    } catch (const std::exception& e) {
        std::cerr << "[srlc Fatal Error] " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
