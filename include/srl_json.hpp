#ifndef SRL_JSON_HPP
#define SRL_JSON_HPP

#include <string>

namespace srl {
class VM;

class JSON {
public:
    static void registerNativeFunctions(VM& vm);
};

} // namespace srl

#endif // SRL_JSON_HPP
