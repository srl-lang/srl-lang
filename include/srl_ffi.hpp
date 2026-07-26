#ifndef SRL_FFI_HPP
#define SRL_FFI_HPP

#include <string>

namespace srl {
class VM;

class FFI {
public:
    static void registerNativeFunctions(VM& vm);
};

} // namespace srl

#endif // SRL_FFI_HPP
