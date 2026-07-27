#include "srl_gui.hpp"
#include "vm.hpp"
#include <iostream>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commdlg.h>
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "user32.lib")
#endif


namespace srl {

std::string GUI::openFileDialog(const std::string& title, const std::string& filterStr) {
#ifdef _WIN32
    char fileName[MAX_PATH] = "";
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetConsoleWindow();
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = sizeof(fileName);

    // Format filter: e.g. "Audio Files\0*.mp3;*.wav\0All Files\0*.*\0\0"
    std::string formattedFilter = filterStr;
    if (formattedFilter.empty()) {
        formattedFilter = "Audio Files (*.mp3, *.wav)\0*.mp3;*.wav\0All Files (*.*)\0*.*\0";
    } else {
        // Replace '|' with '\0'
        for (char& c : formattedFilter) {
            if (c == '|') c = '\0';
        }
        formattedFilter.push_back('\0');
    }

    ofn.lpstrFilter = formattedFilter.c_str();
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = title.c_str();
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn) == TRUE) {
        return std::string(fileName);
    }
#endif
    return "";
}

std::string GUI::saveFileDialog(const std::string& title, const std::string& filterStr) {
#ifdef _WIN32
    char fileName[MAX_PATH] = "";
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetConsoleWindow();
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = sizeof(fileName);

    std::string formattedFilter = filterStr;
    if (formattedFilter.empty()) {
        formattedFilter = "All Files (*.*)\0*.*\0";
    } else {
        for (char& c : formattedFilter) {
            if (c == '|') c = '\0';
        }
        formattedFilter.push_back('\0');
    }

    ofn.lpstrFilter = formattedFilter.c_str();
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = title.c_str();
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

    if (GetSaveFileNameA(&ofn) == TRUE) {
        return std::string(fileName);
    }
#endif
    return "";
}

int GUI::msgBox(const std::string& title, const std::string& message, const std::string& type) {
#ifdef _WIN32
    UINT uType = MB_OK;
    if (type == "warning") uType = MB_OK | MB_ICONWARNING;
    else if (type == "error") uType = MB_OK | MB_ICONERROR;
    else if (type == "question") uType = MB_YESNO | MB_ICONQUESTION;
    else uType = MB_OK | MB_ICONINFORMATION;

    return static_cast<int>(MessageBoxA(GetConsoleWindow(), message.c_str(), title.c_str(), uType));
#else
    std::cout << "[" << title << "] " << message << std::endl;
    return 1;
#endif
}

void GUI::registerNativeFunctions(VM& vm) {
    vm.defineNative("gui_file_dialog_open", [](int argCount, const Value* args) -> Value {
        std::string title = (argCount > 0 && args[0].isString()) ? args[0].asString() : "Open File";
        std::string filter = (argCount > 1 && args[1].isString()) ? args[1].asString() : "";
        return Value(GUI::openFileDialog(title, filter));
    });

    vm.defineNative("gui_file_dialog_save", [](int argCount, const Value* args) -> Value {
        std::string title = (argCount > 0 && args[0].isString()) ? args[0].asString() : "Save File";
        std::string filter = (argCount > 1 && args[1].isString()) ? args[1].asString() : "";
        return Value(GUI::saveFileDialog(title, filter));
    });

    vm.defineNative("gui_msgbox", [](int argCount, const Value* args) -> Value {
        std::string title = (argCount > 0 && args[0].isString()) ? args[0].asString() : "Bilgi";
        std::string msg = (argCount > 1 && args[1].isString()) ? args[1].asString() : "";
        std::string type = (argCount > 2 && args[2].isString()) ? args[2].asString() : "info";
        return Value(static_cast<double>(GUI::msgBox(title, msg, type)));
    });
}

} // namespace srl
