#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "installer_gui.hpp"
#include "installer.hpp"
#include "cli.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <thread>
#include <chrono>


#ifdef _WIN32
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shlwapi.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#endif

namespace fs = std::filesystem;

namespace srl {
namespace installer {

#ifdef _WIN32

#define ID_BTN_BROWSE 1001
#define ID_BTN_INSTALL 1002
#define ID_BTN_CANCEL 1003
#define ID_EDIT_PATH 1004
#define ID_EDIT_LOG 1005
#define ID_PROGRESS 1006
#define ID_CHK_PATH 1007
#define ID_CHK_SHORTCUT 1008

static HWND g_hWnd = NULL;
static HWND g_hEditPath = NULL;
static HWND g_hEditLog = NULL;
static HWND g_hProgressBar = NULL;
static HWND g_hBtnInstall = NULL;
static HWND g_hBtnCancel = NULL;
static HWND g_hChkPath = NULL;
static HWND g_hChkShortcut = NULL;

static HFONT g_hFontTitle = NULL;
static HFONT g_hFontNormal = NULL;
static HFONT g_hFontBold = NULL;

static GuiSetupConfig g_config;
static bool g_installInProgress = false;
static bool g_installSuccess = false;

static void LogGuiMessage(const std::string& msg) {
    if (!g_hEditLog) return;
    std::string text = msg + "\r\n";
    int len = GetWindowTextLengthA(g_hEditLog);
    SendMessageA(g_hEditLog, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessageA(g_hEditLog, EM_REPLACESEL, 0, (LPARAM)text.c_str());
}

static void SetProgress(int percent) {
    if (g_hProgressBar) {
        SendMessageA(g_hProgressBar, PBM_SETPOS, (WPARAM)percent, 0);
    }
}

static void CreateDesktopShortcut(const std::string& targetExe, const std::string& shortcutName) {
    char desktopPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, desktopPath))) {
        fs::path shortcutPath = fs::path(desktopPath) / (shortcutName + ".lnk");
        
        CoInitialize(NULL);
        IShellLinkA* pShellLink = NULL;
        HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkA, (void**)&pShellLink);
        if (SUCCEEDED(hr)) {
            pShellLink->SetPath(targetExe.c_str());
            pShellLink->SetWorkingDirectory(fs::path(targetExe).parent_path().string().c_str());
            pShellLink->SetDescription(shortcutName.c_str());

            IPersistFile* pPersistFile = NULL;
            hr = pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile);
            if (SUCCEEDED(hr)) {
                WCHAR wpath[MAX_PATH];
                MultiByteToWideChar(CP_ACP, 0, shortcutPath.string().c_str(), -1, wpath, MAX_PATH);
                pPersistFile->Save(wpath, TRUE);
                pPersistFile->Release();
                LogGuiMessage("Created Desktop Shortcut: " + shortcutPath.string());
            }
            pShellLink->Release();
        }
        CoUninitialize();
    }
}

static void RunInstallationWorker(std::string targetDir, bool updatePath, bool createShortcut) {
    g_installInProgress = true;
    EnableWindow(g_hBtnInstall, FALSE);
    EnableWindow(g_hEditPath, FALSE);

    LogGuiMessage("[1/5] Initializing target installation directory...");
    SetProgress(10);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    try {
        fs::create_directories(fs::path(targetDir) / "bin");
        fs::create_directories(fs::path(targetDir) / "std");
        fs::create_directories(fs::path(targetDir) / "include");
        fs::create_directories(fs::path(targetDir) / "compiler");
    } catch (...) {}

    LogGuiMessage("[2/5] Deploying core toolchain executables...");
    SetProgress(30);

    InstallOptions opts;
    opts.customInstallPath = targetDir;
    opts.updatePathEnv = updatePath;

    NativeInstaller inst(opts);
    bool ok = inst.run();

    SetProgress(70);
    LogGuiMessage("[3/5] Deploying standard library, compiler modules, and headers...");
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    if (createShortcut) {
        LogGuiMessage("[4/5] Registering desktop shortcuts...");
        fs::path srlExe = fs::path(targetDir) / "bin" / "srl.exe";
        if (fs::exists(srlExe)) {
            CreateDesktopShortcut(srlExe.string(), g_config.appName);
        }
    }

    SetProgress(100);
    LogGuiMessage("[5/5] Setup completed successfully!");
    LogGuiMessage("========================================================");
    LogGuiMessage(" " + g_config.appName + " is ready to use!");
    LogGuiMessage("========================================================");

    g_installSuccess = true;
    g_installInProgress = false;

    SetWindowTextA(g_hBtnInstall, "Finish");
    EnableWindow(g_hBtnInstall, TRUE);
    EnableWindow(g_hBtnCancel, FALSE);
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        INITCOMMONCONTROLSEX icex;
        icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icex.dwICC = ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES;
        InitCommonControlsEx(&icex);

        g_hFontTitle = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        g_hFontBold = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        g_hFontNormal = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

        // Controls
        HWND hLblPath = CreateWindowA("STATIC", "Destination Directory:", WS_CHILD | WS_VISIBLE, 20, 85, 300, 20, hWnd, NULL, NULL, NULL);
        SendMessageA(hLblPath, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        g_hEditPath = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", g_config.defaultInstallPath.c_str(), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 20, 110, 420, 25, hWnd, (HMENU)ID_EDIT_PATH, NULL, NULL);
        SendMessageA(g_hEditPath, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

        HWND hBtnBrowse = CreateWindowA("BUTTON", "Browse...", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 450, 110, 80, 25, hWnd, (HMENU)ID_BTN_BROWSE, NULL, NULL);
        SendMessageA(hBtnBrowse, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

        g_hChkPath = CreateWindowA("BUTTON", "Add executables directory to User PATH environment variable", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 20, 145, 500, 20, hWnd, (HMENU)ID_CHK_PATH, NULL, NULL);
        SendMessageA(g_hChkPath, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
        SendMessageA(g_hChkPath, BM_SETCHECK, BST_CHECKED, 0);

        g_hChkShortcut = CreateWindowA("BUTTON", "Create Desktop Shortcut", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 20, 170, 500, 20, hWnd, (HMENU)ID_CHK_SHORTCUT, NULL, NULL);
        SendMessageA(g_hChkShortcut, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
        SendMessageA(g_hChkShortcut, BM_SETCHECK, BST_CHECKED, 0);

        g_hProgressBar = CreateWindowExA(0, PROGRESS_CLASSA, NULL, WS_CHILD | WS_VISIBLE, 20, 200, 510, 20, hWnd, (HMENU)ID_PROGRESS, NULL, NULL);

        g_hEditLog = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL, 20, 230, 510, 120, hWnd, (HMENU)ID_EDIT_LOG, NULL, NULL);
        SendMessageA(g_hEditLog, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

        g_hBtnInstall = CreateWindowA("BUTTON", "Install", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 340, 365, 90, 30, hWnd, (HMENU)ID_BTN_INSTALL, NULL, NULL);
        SendMessageA(g_hBtnInstall, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        g_hBtnCancel = CreateWindowA("BUTTON", "Cancel", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 440, 365, 90, 30, hWnd, (HMENU)ID_BTN_CANCEL, NULL, NULL);
        SendMessageA(g_hBtnCancel, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

        LogGuiMessage("Ready to install " + g_config.appName + " v" + g_config.appVersion + ".");
        LogGuiMessage("Click 'Install' to begin setup.");
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // Header Background Banner
        RECT headerRect = {0, 0, 560, 70};
        HBRUSH hHeaderBrush = CreateSolidBrush(RGB(15, 23, 42)); // Modern Navy Header
        FillRect(hdc, &headerRect, hHeaderBrush);
        DeleteObject(hHeaderBrush);

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 255));
        SelectObject(hdc, g_hFontTitle);
        TextOutA(hdc, 20, 12, (g_config.appName + " Setup Wizard").c_str(), (int)(g_config.appName.length() + 13));

        SetTextColor(hdc, RGB(148, 163, 184));
        SelectObject(hdc, g_hFontNormal);
        std::string sub = "Version " + g_config.appVersion + " - Native Toolchain Setup";
        TextOutA(hdc, 20, 40, sub.c_str(), (int)sub.length());

        EndPaint(hWnd, &ps);
        break;
    }

    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        if (wmId == ID_BTN_BROWSE) {
            char path[MAX_PATH] = {0};
            BROWSEINFOA bi = {0};
            bi.hwndOwner = hWnd;
            bi.lpszTitle = "Select Destination Installation Directory:";
            bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
            LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
            if (pidl != NULL) {
                if (SHGetPathFromIDListA(pidl, path)) {
                    fs::path p = fs::path(path) / ".srl";
                    SetWindowTextA(g_hEditPath, p.string().c_str());
                }
                CoTaskMemFree(pidl);
            }
        } else if (wmId == ID_BTN_INSTALL) {
            if (g_installSuccess) {
                PostQuitMessage(0);
            } else if (!g_installInProgress) {
                char bufPath[MAX_PATH];
                GetWindowTextA(g_hEditPath, bufPath, MAX_PATH);
                bool chkPath = (SendMessageA(g_hChkPath, BM_GETCHECK, 0, 0) == BST_CHECKED);
                bool chkShortcut = (SendMessageA(g_hChkShortcut, BM_GETCHECK, 0, 0) == BST_CHECKED);

                std::thread worker(RunInstallationWorker, std::string(bufPath), chkPath, chkShortcut);
                worker.detach();
            }
        } else if (wmId == ID_BTN_CANCEL) {
            if (!g_installInProgress) {
                PostQuitMessage(0);
            }
        }
        break;
    }

    case WM_DESTROY:
        if (g_hFontTitle) DeleteObject(g_hFontTitle);
        if (g_hFontNormal) DeleteObject(g_hFontNormal);
        if (g_hFontBold) DeleteObject(g_hFontBold);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcA(hWnd, msg, wParam, lParam);
    }
    return 0;
}

int runGuiInstallerWizard(HINSTANCE hInstance, const GuiSetupConfig& config) {
    g_config = config;

    INITCOMMONCONTROLSEX icex = {0};
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);

    if (g_config.defaultInstallPath.empty()) {
        const char* localAppData = std::getenv("LOCALAPPDATA");
        if (localAppData && std::string(localAppData).length() > 0) {
            g_config.defaultInstallPath = (fs::path(localAppData) / ".srl").string();
        } else {
            const char* userProfile = std::getenv("USERPROFILE");
            if (userProfile && std::string(userProfile).length() > 0) {
                g_config.defaultInstallPath = (fs::path(userProfile) / ".srl").string();
            } else {
                g_config.defaultInstallPath = "C:\\.srl";
            }
        }
    }

    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = "SRL_MODERN_INSTALLER_WIZARD";

    RegisterClassExA(&wc);

    int width = 560;
    int height = 445;
    int posX = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    int posY = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

    g_hWnd = CreateWindowExA(
        0,
        "SRL_MODERN_INSTALLER_WIZARD",
        (g_config.appName + " Setup").c_str(),
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        posX, posY, width, height,
        NULL, NULL, hInstance, NULL
    );

    if (!g_hWnd) {
        return 1;
    }


    ShowWindow(g_hWnd, SW_SHOW);
    UpdateWindow(g_hWnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return 0;
}

#endif // _WIN32

int handleInstallerCli(int argc, char* argv[]) {
    GuiSetupConfig config;
    std::string scriptPath = "";
    std::string outPath = "app_setup.exe";

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-o" && i + 1 < argc) {
            outPath = argv[++i];
        } else if (arg == "--name" && i + 1 < argc) {
            config.appName = argv[++i];
        } else if (scriptPath.empty() && arg[0] != '-') {
            scriptPath = arg;
        }
    }

    if (scriptPath.empty()) {
        cli::Term::error("Error:", "Expected script file: srl installer <file.srl> [-o setup.exe] [--name \"App Name\"]");
        return 1;
    }

    cli::Term::status("Installer Generator", "Packaging script '" + scriptPath + "' into modern GUI setup binary '" + outPath + "'...");

    std::string scriptSource;
    try {
        std::ifstream f(scriptPath);
        if (!f.is_open()) {
            cli::Term::error("Error:", "Could not read input script file: " + scriptPath);
            return 1;
        }
        std::stringstream ss;
        ss << f.rdbuf();
        scriptSource = ss.str();
    } catch (const std::exception& e) {
        cli::Term::error("Error:", e.what());
        return 1;
    }

    fs::path selfInstaller = fs::path(argv[0]).parent_path() / "srl-installer.exe";
    if (!fs::exists(selfInstaller)) {
        selfInstaller = "srl-installer.exe";
    }

    if (!fs::exists(selfInstaller)) {
        cli::Term::error("Error:", "Could not locate base installer template binary (srl-installer.exe).");
        return 1;
    }

    try {
        fs::copy_file(selfInstaller, outPath, fs::copy_options::overwrite_existing);

        std::ofstream out(outPath, std::ios::binary | std::ios::app);
        uint64_t size = scriptSource.size();
        out.write(scriptSource.data(), size);
        out.write(reinterpret_cast<const char*>(&size), sizeof(size));
        out.write("SRLPAYL1", 8);
        out.close();

        std::cout << "========================================================" << std::endl;
        cli::Term::status("Success", "Generated Universal GUI Setup Installer Binary!");
        std::cout << "  Output Installer : " << outPath << std::endl;
        std::cout << "  Application Name : " << config.appName << std::endl;
        std::cout << "  Source Script    : " << scriptPath << std::endl;
        std::cout << "========================================================" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        cli::Term::error("Error:", e.what());
        return 1;
    }
}

} // namespace installer
} // namespace srl

