#include "installer.hpp"
#include "cli.hpp"
#include <iostream>
#include <filesystem>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif

namespace fs = std::filesystem;

namespace srl {
namespace installer {

NativeInstaller::NativeInstaller(const InstallOptions& opts) : m_opts(opts) {}

std::string NativeInstaller::getTargetDir() {
    if (!m_opts.customInstallPath.empty()) {
        return m_opts.customInstallPath;
    }

#ifdef _WIN32
    const char* localAppData = std::getenv("LOCALAPPDATA");
    if (localAppData && std::string(localAppData).length() > 0) {
        return (fs::path(localAppData) / ".srl").string();
    }
    const char* userProfile = std::getenv("USERPROFILE");
    if (userProfile && std::string(userProfile).length() > 0) {
        return (fs::path(userProfile) / ".srl").string();
    }
    return "C:\\.srl";
#else
    const char* home = std::getenv("HOME");
    if (home && std::string(home).length() > 0) {
        return (fs::path(home) / ".srl").string();
    }
    return "/tmp/.srl";
#endif
}

bool NativeInstaller::createDirectories(const std::string& targetDir) {
    try {
        fs::create_directories(fs::path(targetDir) / "bin");
        fs::create_directories(fs::path(targetDir) / "std");
        fs::create_directories(fs::path(targetDir) / "include");
        fs::create_directories(fs::path(targetDir) / "compiler");
        return true;
    } catch (const std::exception& ex) {
        std::cerr << "[Installer Error] Failed to create directories: " << ex.what() << std::endl;
        return false;
    }
}

bool NativeInstaller::copyToolchainFiles(const std::string& targetDir) {
    fs::path targetBin = fs::path(targetDir) / "bin";
    fs::path targetStd = fs::path(targetDir) / "std";
    fs::path targetInclude = fs::path(targetDir) / "include";
    fs::path targetCompiler = fs::path(targetDir) / "compiler";

    // 1. Copy srl executable
    std::string srlExeName = "srl.exe";
#ifndef _WIN32
    srlExeName = "srl";
#endif

    std::vector<fs::path> candidateSrlPaths = {
        fs::path(srlExeName),
        fs::path("build") / "Release" / srlExeName,
        fs::path("build") / "Debug" / srlExeName,
        fs::path("build") / srlExeName
    };

    bool copiedSrl = false;
    for (const auto& p : candidateSrlPaths) {
        if (fs::exists(p)) {
            fs::copy_file(p, targetBin / srlExeName, fs::copy_options::overwrite_existing);
            cli::Term::status("Deploy", "Deployed " + p.string() + " -> " + (targetBin / srlExeName).string());
            copiedSrl = true;
            break;
        }
    }

    if (!copiedSrl) {
        cli::Term::warn("Deploy", "Could not locate compiled srl executable binary in build directories.");
    }

    // 2. Copy srlc executable if available
    std::string srlcExeName = "srlc.exe";
#ifndef _WIN32
    srlcExeName = "srlc";
#endif
    std::vector<fs::path> candidateSrlcPaths = {
        fs::path(srlcExeName),
        fs::path("build") / "Release" / srlcExeName,
        fs::path("build") / "Debug" / srlcExeName,
        fs::path("build") / srlcExeName
    };

    for (const auto& p : candidateSrlcPaths) {
        if (fs::exists(p)) {
            fs::copy_file(p, targetBin / srlcExeName, fs::copy_options::overwrite_existing);
            cli::Term::status("Deploy", "Deployed " + p.string() + " -> " + (targetBin / srlcExeName).string());
            break;
        }
    }

    // 3. Copy directories
    if (fs::exists("std")) {
        fs::copy("std", targetStd, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        cli::Term::status("Deploy", "Deployed stdlib modules to " + targetStd.string());
    }
    if (fs::exists("include")) {
        fs::copy("include", targetInclude, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        cli::Term::status("Deploy", "Deployed C++ header files to " + targetInclude.string());
    }
    if (fs::exists("compiler")) {
        fs::copy("compiler", targetCompiler, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        cli::Term::status("Deploy", "Deployed self-hosted compiler files to " + targetCompiler.string());
    }

    return true;
}

bool NativeInstaller::updateSystemPath(const std::string& binDir) {
#ifdef _WIN32
    HKEY hKey;
    LONG lRes = RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_READ | KEY_WRITE, &hKey);
    if (lRes == ERROR_SUCCESS) {
        char currentPath[8192] = {0};
        DWORD pathLen = sizeof(currentPath);
        DWORD type = REG_EXPAND_SZ;
        if (RegQueryValueExA(hKey, "Path", NULL, &type, (LPBYTE)currentPath, &pathLen) != ERROR_SUCCESS) {
            type = REG_EXPAND_SZ;
        }

        std::string pathStr(currentPath);
        if (pathStr.find(binDir) == std::string::npos) {
            if (!pathStr.empty() && pathStr.back() != ';') {
                pathStr += ";";
            }
            pathStr += binDir;
            RegSetValueExA(hKey, "Path", 0, type, (const BYTE*)pathStr.c_str(), (DWORD)(pathStr.length() + 1));
            cli::Term::status("Environment", "Added '" + binDir + "' to User PATH environment variable.");

            // Notify windows environment change
            DWORD_PTR dwResult;
            SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Environment", SMTO_ABORTIFHUNG, 5000, &dwResult);

        } else {
            cli::Term::status("Environment", "'" + binDir + "' is already present in User PATH.");
        }
        RegCloseKey(hKey);
    }
#else
    cli::Term::status("Environment", "Target binary path: " + binDir + " (Ensure this is exported in ~/.bashrc or ~/.zshrc)");
#endif
    return true;
}

void NativeInstaller::verifyInstallation(const std::string& binDir) {
    fs::path targetExe = fs::path(binDir) / "srl.exe";
#ifndef _WIN32
    targetExe = fs::path(binDir) / "srl";
#endif
    if (fs::exists(targetExe)) {
        cli::Term::status("Verification", "Toolchain installation verified cleanly.");
    }
}

bool NativeInstaller::run() {
    std::cout << "========================================================" << std::endl;
    std::cout << " SRL (Serial Run Language) Native Standalone Installer" << std::endl;
    std::cout << " Version 0.3.2 (Custom Native Installer Engine)" << std::endl;
    std::cout << "========================================================" << std::endl;

    std::string targetDir = getTargetDir();
    cli::Term::status("Setup", "Installation Directory: " + targetDir);

    if (!createDirectories(targetDir)) {
        return false;
    }

    copyToolchainFiles(targetDir);

    if (m_opts.updatePathEnv) {
        updateSystemPath((fs::path(targetDir) / "bin").string());
    }

    verifyInstallation((fs::path(targetDir) / "bin").string());

    std::cout << std::endl;
    std::cout << "========================================================" << std::endl;
    cli::Term::status("Success", "SRL Toolchain Installed Successfully!");
    std::cout << "========================================================" << std::endl;
    return true;
}

int runInstallerCli(int argc, char* argv[]) {
    InstallOptions opts;
    if (argc >= 3) {
        opts.customInstallPath = argv[2];
    }
    NativeInstaller inst(opts);
    return inst.run() ? 0 : 1;
}

} // namespace installer
} // namespace srl

#include "installer_gui.hpp"

// Standalone installer entry point when built as srl-installer executable
#ifdef SRL_STANDALONE_INSTALLER
int main(int argc, char* argv[]) {
    bool forceCli = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--cli") {
            forceCli = true;
            break;
        }
    }

#ifdef _WIN32
    if (!forceCli) {
        srl::installer::GuiSetupConfig config;
        return srl::installer::runGuiInstallerWizard(GetModuleHandle(NULL), config);
    }
#endif

    srl::cli::Term::enableAnsi();
    return srl::installer::runInstallerCli(argc, argv);
}
#endif

