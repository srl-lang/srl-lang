#ifndef SRL_ENV_HPP
#define SRL_ENV_HPP

#include "value.hpp"
#include <unordered_map>
#include <string>

namespace srl {

class Environment {
public:
    void defineGlobal(const std::string& name, const Value& value) {
        // PRESERVE STATE: If global variable already exists during hot reload, KEEP IT!
        if (globals_.find(name) == globals_.end()) {
            globals_[name] = value;
        }
    }

    void setGlobal(const std::string& name, const Value& value) {
        globals_[name] = value;
    }

    bool getGlobal(const std::string& name, Value& value) const {
        auto it = globals_.find(name);
        if (it != globals_.end()) {
            value = it->second;
            return true;
        }
        return false;
    }

    const std::unordered_map<std::string, Value>& getGlobals() const {
        return globals_;
    }

private:
    std::unordered_map<std::string, Value> globals_;
};

} // namespace srl

#endif // SRL_ENV_HPP
