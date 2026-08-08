// src/srl_gui.cpp - Native C++ FFI Extensions for SRL IDE Subsystems
#include "srl_gui.hpp"
#include "vm.hpp"
#include <iostream>
#include <sstream>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

namespace srl {

static std::atomic<uint64_t> s_nextId{1000};
static std::mutex s_guiMutex;

static std::map<uint64_t, GuiWindowHandle> s_windows;
static std::map<uint64_t, GuiSplitterHandle> s_splitters;
static std::map<uint64_t, GuiDockPanelHandle> s_docks;
static std::map<uint64_t, GuiEditorHandle> s_editors;
static std::map<uint64_t, GuiTreeViewHandle> s_trees;
static std::map<uint64_t, ProcessPipeHandle> s_processes;
static std::map<uint64_t, FileWatcherHandle> s_watchers;

void GUI::registerNativeFunctions(VM& vm) {
    // Window Management FFI
    vm.defineNative("gui_window_create", [](int argCount, const Value* args) -> Value {
        if (argCount < 3 || !args[0].isString() || !args[1].isNumber() || !args[2].isNumber()) {
            return Value();
        }
        std::lock_guard<std::mutex> lock(s_guiMutex);
        uint64_t id = s_nextId++;
        GuiWindowHandle win;
        win.id = id;
        win.title = args[0].asString();
        win.width = static_cast<int>(args[1].asNumber());
        win.height = static_cast<int>(args[2].asNumber());
        win.visible = true;
        s_windows[id] = win;
        return Value(static_cast<double>(id));
    });

    vm.defineNative("gui_window_show", [](int argCount, const Value* args) -> Value {
        if (argCount < 1 || !args[0].isNumber()) return Value(false);
        std::lock_guard<std::mutex> lock(s_guiMutex);
        uint64_t id = static_cast<uint64_t>(args[0].asNumber());
        if (s_windows.find(id) != s_windows.end()) {
            s_windows[id].visible = true;
            return Value(true);
        }
        return Value(false);
    });

    // Resizable Splitter FFI
    vm.defineNative("gui_splitter_create", [](int argCount, const Value* args) -> Value {
        std::string orient = "horizontal";
        if (argCount >= 1 && args[0].isString()) {
            orient = args[0].asString();
        }
        std::lock_guard<std::mutex> lock(s_guiMutex);
        uint64_t id = s_nextId++;
        GuiSplitterHandle s;
        s.id = id;
        s.parentId = 0;
        s.orientation = orient;
        s_splitters[id] = s;
        return Value(static_cast<double>(id));
    });

    // Dock Panel FFI
    vm.defineNative("gui_dock_panel_create", [](int argCount, const Value* args) -> Value {
        std::string area = "left";
        std::string title = "Panel";
        if (argCount >= 1 && args[0].isString()) area = args[0].asString();
        if (argCount >= 2 && args[1].isString()) title = args[1].asString();

        std::lock_guard<std::mutex> lock(s_guiMutex);
        uint64_t id = s_nextId++;
        GuiDockPanelHandle d;
        d.id = id;
        d.area = area;
        d.title = title;
        d.visible = true;
        s_docks[id] = d;
        return Value(static_cast<double>(id));
    });

    // Text Editor FFI
    vm.defineNative("gui_editor_create", [](int argCount, const Value* args) -> Value {
        uint64_t parentId = 0;
        if (argCount >= 1 && args[0].isNumber()) parentId = static_cast<uint64_t>(args[0].asNumber());

        std::lock_guard<std::mutex> lock(s_guiMutex);
        uint64_t id = s_nextId++;
        GuiEditorHandle ed;
        ed.id = id;
        ed.parentId = parentId;
        ed.fontName = "Consolas";
        ed.fontSize = 12;
        s_editors[id] = ed;
        return Value(static_cast<double>(id));
    });

    vm.defineNative("gui_editor_set_text", [](int argCount, const Value* args) -> Value {
        if (argCount < 2 || !args[0].isNumber() || !args[1].isString()) return Value(false);
        uint64_t id = static_cast<uint64_t>(args[0].asNumber());
        std::lock_guard<std::mutex> lock(s_guiMutex);
        if (s_editors.find(id) != s_editors.end()) {
            s_editors[id].content = args[1].asString();
            return Value(true);
        }
        return Value(false);
    });

    vm.defineNative("gui_editor_get_text", [](int argCount, const Value* args) -> Value {
        if (argCount < 1 || !args[0].isNumber()) return Value("");
        uint64_t id = static_cast<uint64_t>(args[0].asNumber());
        std::lock_guard<std::mutex> lock(s_guiMutex);
        if (s_editors.find(id) != s_editors.end()) {
            return Value(s_editors[id].content);
        }
        return Value("");
    });

    vm.defineNative("gui_editor_set_font", [](int argCount, const Value* args) -> Value {
        if (argCount < 3 || !args[0].isNumber() || !args[1].isString() || !args[2].isNumber()) return Value(false);
        uint64_t id = static_cast<uint64_t>(args[0].asNumber());
        std::lock_guard<std::mutex> lock(s_guiMutex);
        if (s_editors.find(id) != s_editors.end()) {
            s_editors[id].fontName = args[1].asString();
            s_editors[id].fontSize = static_cast<int>(args[2].asNumber());
            return Value(true);
        }
        return Value(false);
    });

    // Tree View FFI
    vm.defineNative("gui_tree_view_create", [](int argCount, const Value* args) -> Value {
        uint64_t parentId = 0;
        if (argCount >= 1 && args[0].isNumber()) parentId = static_cast<uint64_t>(args[0].asNumber());

        std::lock_guard<std::mutex> lock(s_guiMutex);
        uint64_t id = s_nextId++;
        GuiTreeViewHandle tv;
        tv.id = id;
        tv.parentId = parentId;
        s_trees[id] = tv;
        return Value(static_cast<double>(id));
    });

    vm.defineNative("gui_tree_view_add_node", [](int argCount, const Value* args) -> Value {
        if (argCount < 2 || !args[0].isNumber() || !args[1].isString()) return Value(false);
        uint64_t id = static_cast<uint64_t>(args[0].asNumber());
        std::lock_guard<std::mutex> lock(s_guiMutex);
        if (s_trees.find(id) != s_trees.end()) {
            s_trees[id].nodes.push_back(args[1].asString());
            return Value(true);
        }
        return Value(false);
    });

    // Asynchronous Process Pipe FFI
    vm.defineNative("gui_process_run", [](int argCount, const Value* args) -> Value {
        if (argCount < 1 || !args[0].isString()) return Value();
        std::string cmd = args[0].asString();
        uint64_t id = s_nextId++;

        ProcessPipeHandle proc;
        proc.id = id;
        proc.command = cmd;
        proc.running = true;
        proc.exitCode = 0;

        // Launch process using system command runner thread
        std::thread([id, cmd]() {
            std::string outBuf;
            std::string errBuf;
            int code = 0;

#if defined(_WIN32)
            FILE* pipe = _popen((cmd + " 2>&1").c_str(), "r");
            if (pipe) {
                char buffer[256];
                while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                    outBuf += buffer;
                }
                code = _pclose(pipe);
            } else {
                code = -1;
            }
#else
            FILE* pipe = popen((cmd + " 2>&1").c_str(), "r");
            if (pipe) {
                char buffer[256];
                while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                    outBuf += buffer;
                }
                code = pclose(pipe);
            } else {
                code = -1;
            }
#endif

            std::lock_guard<std::mutex> lock(s_guiMutex);
            if (s_processes.find(id) != s_processes.end()) {
                s_processes[id].stdoutBuffer = outBuf;
                s_processes[id].exitCode = code;
                s_processes[id].running = false;
            }
        }).detach();

        std::lock_guard<std::mutex> lock(s_guiMutex);
        s_processes[id] = proc;
        return Value(static_cast<double>(id));
    });

    vm.defineNative("gui_process_poll", [](int argCount, const Value* args) -> Value {
        if (argCount < 1 || !args[0].isNumber()) return Value();
        uint64_t id = static_cast<uint64_t>(args[0].asNumber());

        std::lock_guard<std::mutex> lock(s_guiMutex);
        auto it = s_processes.find(id);
        if (it != s_processes.end()) {
            MapPtr mapObj = std::make_shared<std::unordered_map<std::string, Value>>();
            (*mapObj)["id"] = Value(static_cast<double>(it->second.id));
            (*mapObj)["running"] = Value(it->second.running);
            (*mapObj)["exit_code"] = Value(static_cast<double>(it->second.exitCode));
            (*mapObj)["stdout"] = Value(it->second.stdoutBuffer);
            (*mapObj)["stderr"] = Value(it->second.stderrBuffer);
            return Value(mapObj);
        }
        return Value();
    });

    // File System Watcher FFI
    vm.defineNative("gui_file_watcher_create", [](int argCount, const Value* args) -> Value {
        if (argCount < 1 || !args[0].isString()) return Value();
        std::string path = args[0].asString();
        uint64_t id = s_nextId++;

        FileWatcherHandle w;
        w.id = id;
        w.watchPath = path;
        w.active = true;

        std::lock_guard<std::mutex> lock(s_guiMutex);
        s_watchers[id] = w;
        return Value(static_cast<double>(id));
    });

    vm.defineNative("gui_file_watcher_poll", [](int argCount, const Value* args) -> Value {
        if (argCount < 1 || !args[0].isNumber()) return Value();
        uint64_t id = static_cast<uint64_t>(args[0].asNumber());

        std::lock_guard<std::mutex> lock(s_guiMutex);
        auto it = s_watchers.find(id);
        if (it != s_watchers.end()) {
            ArrayPtr arrObj = std::make_shared<std::vector<Value>>();
            for (const auto& f : it->second.modifiedFiles) {
                arrObj->push_back(Value(f));
            }
            it->second.modifiedFiles.clear();
            return Value(arrObj);
        }
        return Value();
    });
}

} // namespace srl
