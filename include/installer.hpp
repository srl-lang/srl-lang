#pragma once

#include <string>

namespace srl {
namespace installer {

struct InstallOptions {
    std::string customInstallPath = "";
    bool verbose = true;
    bool updatePathEnv = true;
};

class NativeInstaller {
public:
    NativeInstaller(const InstallOptions& opts = InstallOptions());
    bool run();

private:
    InstallOptions m_opts;
    std::string getTargetDir();
    bool createDirectories(const std::string& targetDir);
    bool copyToolchainFiles(const std::string& targetDir);
    bool updateSystemPath(const std::string& binDir);
    void verifyInstallation(const std::string& binDir);
};

int runInstallerCli(int argc, char* argv[]);
int handleInstallerCli(int argc, char* argv[]);


} // namespace installer
} // namespace srl
