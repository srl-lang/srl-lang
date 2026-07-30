#ifndef SRL_FFI_HPP
#define SRL_FFI_HPP

#include <string>

namespace srl {
class VM;

class FFI {
public:
    static void registerNativeFunctions(VM& vm);

    // Shared helper: load and initialise a plugin DLL/SO at runtime.
    // Supports both v1 (srl_module_init(void*)) and v2 (srl_module_init_v2(SRL_PluginCtx*)) ABI.
    // Idempotent: calling with the same path twice is a no-op.
    static bool loadPlugin(VM& vm, const std::string& path, const std::string& displayName = "");
};

} // namespace srl

#endif // SRL_FFI_HPP
