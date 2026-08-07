#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "srl_db.hpp"
#include "vm.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <filesystem>

#include <mutex>

namespace srl {

struct DbStore {
    std::string filepath;
    std::unordered_map<std::string, std::string> data;
    bool dirty = false;
};

static std::unordered_map<double, DbStore> openStores;
static double dbCounter = 1.0;
static std::mutex g_dbMutex;

static std::string dbEscape(const std::string& s) {
    std::string res;
    for (char c : s) {
        if (c == '\\') res += "\\\\";
        else if (c == '\n') res += "\\n";
        else if (c == '\r') res += "\\r";
        else if (c == '=') res += "\\e";
        else res += c;
    }
    return res;
}

static std::string dbUnescape(const std::string& s) {
    std::string res;
    for (size_t i = 0; i < s.length(); ++i) {
        if (s[i] == '\\' && i + 1 < s.length()) {
            char next = s[++i];
            if (next == 'n') res += '\n';
            else if (next == 'r') res += '\r';
            else if (next == 'e') res += '=';
            else res += next;
        } else {
            res += s[i];
        }
    }
    return res;
}

static void saveStore(DbStore& store) {
    if (!store.dirty) return;
    std::ofstream file(store.filepath);
    if (file.is_open()) {
        for (const auto& [k, v] : store.data) {
            file << dbEscape(k) << "=" << dbEscape(v) << "\n";
        }
        store.dirty = false;
    }
}

static void loadStore(DbStore& store) {
    store.data.clear();
    store.dirty = false;
    std::ifstream file(store.filepath);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            size_t eqPos = line.find('=');
            if (eqPos != std::string::npos) {
                std::string k = dbUnescape(line.substr(0, eqPos));
                std::string v = dbUnescape(line.substr(eqPos + 1));
                store.data[k] = v;
            }
        }
    }
}


void DB::registerNativeFunctions(VM& vm) {
    // db_open(path)
    vm.defineNative("db_open", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            std::string path = args[0].asString();
            std::lock_guard<std::mutex> lock(g_dbMutex);
            double id = dbCounter++;
            DbStore store;
            store.filepath = path;
            loadStore(store);
            openStores[id] = store;
            return Value(id);
        }
        return Value(0.0);
    });

    // db_set(db, key, val)
    vm.defineNative("db_set", [](int argCount, const Value* args) -> Value {
        if (argCount >= 3 && args[0].isNumber() && args[1].isString()) {
            double id = args[0].asNumber();
            std::string key = args[1].asString();
            std::string val = args[2].toString();

            std::lock_guard<std::mutex> lock(g_dbMutex);
            auto it = openStores.find(id);
            if (it != openStores.end()) {
                it->second.data[key] = val;
                it->second.dirty = true;
                return Value(true);
            }
        }
        return Value(false);
    });

    // db_get(db, key)
    vm.defineNative("db_get", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isNumber() && args[1].isString()) {
            double id = args[0].asNumber();
            std::string key = args[1].asString();

            std::lock_guard<std::mutex> lock(g_dbMutex);
            auto it = openStores.find(id);
            if (it != openStores.end()) {
                auto dataIt = it->second.data.find(key);
                if (dataIt != it->second.data.end()) {
                    return Value(dataIt->second);
                }
            }
        }
        return Value();
    });

    // db_has(db, key)
    vm.defineNative("db_has", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isNumber() && args[1].isString()) {
            double id = args[0].asNumber();
            std::string key = args[1].asString();

            std::lock_guard<std::mutex> lock(g_dbMutex);
            auto it = openStores.find(id);
            if (it != openStores.end()) {
                return Value(it->second.data.count(key) > 0);
            }
        }
        return Value(false);
    });

    // db_delete(db, key)
    vm.defineNative("db_delete", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isNumber() && args[1].isString()) {
            double id = args[0].asNumber();
            std::string key = args[1].asString();

            std::lock_guard<std::mutex> lock(g_dbMutex);
            auto it = openStores.find(id);
            if (it != openStores.end()) {
                it->second.data.erase(key);
                it->second.dirty = true;
                return Value(true);
            }
        }
        return Value(false);
    });

    // db_sync(db)
    vm.defineNative("db_sync", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) {
            double id = args[0].asNumber();
            std::lock_guard<std::mutex> lock(g_dbMutex);
            auto it = openStores.find(id);
            if (it != openStores.end()) {
                saveStore(it->second);
                return Value(true);
            }
        }
        return Value(false);
    });

    // db_close(db)
    vm.defineNative("db_close", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) {
            double id = args[0].asNumber();
            std::lock_guard<std::mutex> lock(g_dbMutex);
            auto it = openStores.find(id);
            if (it != openStores.end()) {
                saveStore(it->second);
                openStores.erase(it);
                return Value(true);
            }
        }
        return Value(false);
    });
}

} // namespace srl
