#ifndef SRL_SYS_HPP
#define SRL_SYS_HPP

#include <string>

namespace srl {
class VM;

class SYS {
public:
    static void registerNativeFunctions(VM& vm);
};

} // namespace srl

#endif // SRL_SYS_HPP
