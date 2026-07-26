#ifndef SRL_CRYPTO_HPP
#define SRL_CRYPTO_HPP

#include <string>

namespace srl {
class VM;

class CRYPTO {
public:
    static void registerNativeFunctions(VM& vm);
};

} // namespace srl

#endif // SRL_CRYPTO_HPP
