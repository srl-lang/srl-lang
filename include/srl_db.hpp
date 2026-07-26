#ifndef SRL_DB_HPP
#define SRL_DB_HPP

#include <string>

namespace srl {
class VM;

class DB {
public:
    static void registerNativeFunctions(VM& vm);
};

} // namespace srl

#endif // SRL_DB_HPP
