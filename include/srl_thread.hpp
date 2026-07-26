#ifndef SRL_THREAD_HPP
#define SRL_THREAD_HPP

#include <string>

namespace srl {
class VM;

class THREAD {
public:
    static void registerNativeFunctions(VM& vm);
};

} // namespace srl

#endif // SRL_THREAD_HPP
