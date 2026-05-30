#pragma once

#include <exception>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace haru::test {

struct TestCase {
    std::string name;
    std::function<void()> run;
};

class Registry {
public:
    static Registry& instance();

    void add(std::string name, std::function<void()> run);
    int runAll(std::ostream& out);

private:
    std::vector<TestCase> tests_;
};

class AssertionFailure final : public std::runtime_error {
public:
    explicit AssertionFailure(const std::string& message);
};

void fail(const char* expression, const char* file, int line, const std::string& detail);

template <typename Left, typename Right>
void expectEqual(const Left& left,
                 const Right& right,
                 const char* leftExpression,
                 const char* rightExpression,
                 const char* file,
                 int line) {
    if (!(left == right)) {
        std::ostringstream detail;
        detail << leftExpression << " != " << rightExpression;
        fail("EXPECT_EQ", file, line, detail.str());
    }
}

} // namespace haru::test

#define HARU_TEST(name)                                                         \
    static void name();                                                         \
    namespace {                                                                 \
    struct name##Registrar {                                                    \
        name##Registrar() {                                                     \
            ::haru::test::Registry::instance().add(#name, name);                \
        }                                                                       \
    };                                                                          \
    static name##Registrar name##registrar;                                     \
    }                                                                           \
    static void name()

#define HARU_EXPECT_TRUE(expression)                                            \
    do {                                                                        \
        if (!(expression)) {                                                    \
            ::haru::test::fail(#expression, __FILE__, __LINE__, "was false");  \
        }                                                                       \
    } while (false)

#define HARU_EXPECT_FALSE(expression)                                           \
    do {                                                                        \
        if (expression) {                                                       \
            ::haru::test::fail(#expression, __FILE__, __LINE__, "was true");   \
        }                                                                       \
    } while (false)

#define HARU_EXPECT_EQ(left, right)                                             \
    do {                                                                        \
        ::haru::test::expectEqual((left), (right), #left, #right, __FILE__,     \
                                  __LINE__);                                    \
    } while (false)
