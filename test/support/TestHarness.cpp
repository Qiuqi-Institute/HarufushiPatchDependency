#include "support/TestHarness.hpp"

namespace haru::test {

Registry& Registry::instance() {
    static Registry registry;
    return registry;
}

void Registry::add(std::string name, std::function<void()> run) {
    tests_.push_back(TestCase{std::move(name), std::move(run)});
}

int Registry::runAll(std::ostream& out) {
    int failures = 0;
    for (const auto& test : tests_) {
        try {
            test.run();
            out << "[PASS] " << test.name << '\n';
        } catch (const std::exception& error) {
            ++failures;
            out << "[FAIL] " << test.name << ": " << error.what() << '\n';
        }
    }

    out << tests_.size() << " tests, " << failures << " failures\n";
    return failures == 0 ? 0 : 1;
}

AssertionFailure::AssertionFailure(const std::string& message)
    : std::runtime_error(message) {}

void fail(const char* expression, const char* file, int line, const std::string& detail) {
    std::ostringstream message;
    message << file << ':' << line << " assertion failed: " << expression;
    if (!detail.empty()) {
        message << " (" << detail << ')';
    }
    throw AssertionFailure(message.str());
}

} // namespace haru::test
