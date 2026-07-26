#ifndef SRL_GUI_HPP
#define SRL_GUI_HPP

#include <string>

namespace srl {
class VM;

class GUI {
public:
    static void registerNativeFunctions(VM& vm);

    static std::string openFileDialog(const std::string& title, const std::string& filter);
    static std::string saveFileDialog(const std::string& title, const std::string& filter);
    static int msgBox(const std::string& title, const std::string& message, const std::string& type = "info");
};

} // namespace srl

#endif // SRL_GUI_HPP
