#include <iostream>
#include "tests/test_kvstore.cpp"
#include "src/store/kvstore.h"
#include "src/client/Trivial.h"
int main() {
    Trivial t();
    KVstore kv = KVstore();
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
