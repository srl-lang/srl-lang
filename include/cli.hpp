#pragma once

#include <string>
#include <vector>

namespace srl {
namespace cli {

struct Term {
    static void enableAnsi();
    static std::string bold(const std::string& text);
    static std::string green(const std::string& text);
    static std::string red(const std::string& text);
    static std::string yellow(const std::string& text);
    static std::string cyan(const std::string& text);
    static std::string gray(const std::string& text);

    static void status(const std::string& header, const std::string& message, bool isSuccess = true);
    static void error(const std::string& header, const std::string& message);
    static void warn(const std::string& header, const std::string& message);
};

int handleNew(int argc, char* argv[]);
int handleInit(int argc, char* argv[]);
int handleBuild(int argc, char* argv[]);
int handleRun(int argc, char* argv[]);
int handleCheck(int argc, char* argv[]);
int handleTest(int argc, char* argv[]);
int handleFmt(int argc, char* argv[]);
int handleClean(int argc, char* argv[]);
int handleAdd(int argc, char* argv[]);
int handleRemove(int argc, char* argv[]);
int handleDoc(int argc, char* argv[]);

void printHelp();
void printVersion();

} // namespace cli
} // namespace srl
