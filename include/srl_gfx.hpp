#ifndef SRL_GFX_HPP
#define SRL_GFX_HPP

#include <string>

namespace srl {
class VM;

class GFX {
public:
    static void registerNativeFunctions(VM& vm);
};

} // namespace srl

#endif // SRL_GFX_HPP
