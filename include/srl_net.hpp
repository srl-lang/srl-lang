#ifndef SRL_NET_HPP
#define SRL_NET_HPP

#include <string>

namespace srl {
class VM;

class NET {
public:
    static void registerNativeFunctions(VM& vm);
};

} // namespace srl

#endif // SRL_NET_HPP
