#ifndef SRL_ENV_HPP
#define SRL_ENV_HPP

#include "value.hpp"
#include <unordered_map>
#include <unordered_set>
#include <string>

namespace srl {

class Environment {
public:
    void defineGlobal(const std::string& name, const Value& value, bool isConst = false) {
        if (globals_.find(name) == globals_.end() || isConst) {
            globals_[name] = value;
        }
        if (isConst) {
            constants_.insert(name);
        }
    }

    bool setGlobal(const std::string& name, const Value& value) {
        if (constants_.find(name) != constants_.end()) {
            return false; // Cannot reassign constant variable
        }
        globals_[name] = value;
        return true;
    }

    bool isConst(const std::string& name) const {
        return constants_.find(name) != constants_.end();
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
    std::unordered_set<std::string> constants_;
};

} // namespace srl

#endif // SRL_ENV_HPP
