#ifndef SRL_GUI_HPP
#define SRL_GUI_HPP

#include "value.hpp"
#include <string>
#include <vector>
#include <functional>
#include <map>

namespace srl {

class VM;

struct GuiWindowHandle {
    uint64_t id;
    std::string title;
    int width;
    int height;
    bool visible;
};

struct GuiSplitterHandle {
    uint64_t id;
    uint64_t parentId;
    std::string orientation;
    std::vector<uint64_t> childPanels;
};

struct GuiDockPanelHandle {
    uint64_t id;
    std::string area;
    std::string title;
    bool visible;
};

struct GuiEditorHandle {
    uint64_t id;
    uint64_t parentId;
    std::string content;
    std::string fontName;
    int fontSize;
    std::vector<std::pair<int, std::string>> tokenStyles;
};

struct GuiTreeViewHandle {
    uint64_t id;
    uint64_t parentId;
    std::vector<std::string> nodes;
};

struct ProcessPipeHandle {
    uint64_t id;
    std::string command;
    bool running;
    int exitCode;
    std::string stdoutBuffer;
    std::string stderrBuffer;
};

struct FileWatcherHandle {
    uint64_t id;
    std::string watchPath;
    bool active;
    std::vector<std::string> modifiedFiles;
};

class GUI {
public:
    static void registerNativeFunctions(VM& vm);
};

} // namespace srl

#endif // SRL_GUI_HPP
