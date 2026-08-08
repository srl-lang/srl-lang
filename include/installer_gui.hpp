#pragma once

#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <commctrl.h>
#endif

namespace srl {
namespace installer {

struct GuiSetupConfig {
    std::string appName = "SRL Language Toolchain";
    std::string appVersion = "0.3.3";
    std::string defaultInstallPath = "";
    std::string payloadSource = "";
    bool createDesktopShortcut = true;
    bool updatePathEnv = true;
};

#ifdef _WIN32
int runGuiInstallerWizard(HINSTANCE hInstance, const GuiSetupConfig& config);
#endif

int handleInstallerCli(int argc, char* argv[]);

} // namespace installer
} // namespace srl
