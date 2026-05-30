#include "support/TestHarness.hpp"

int main() {
    return haru::test::Registry::instance().runAll(std::cout);
}
