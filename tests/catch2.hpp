#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <functional>
#include <stdexcept>
#include <exception>

struct TestCase {
    std::string name;
    std::string tag;
    std::function<void()> func;
};

inline std::vector<TestCase>& test_registry() {
    static std::vector<TestCase> registry;
    return registry;
}

struct AutoRegister {
    AutoRegister(const char* name, const char* tag, std::function<void()> func) {
        test_registry().push_back({name, tag, func});
    }
};

#define CONCAT_IMPL(a, b) a ## b
#define CONCAT(a, b) CONCAT_IMPL(a, b)

#define TEST_CASE(name, tag) \
    static void CONCAT(test_func_, __LINE__)(); \
    static AutoRegister CONCAT(reg_, __LINE__)(name, tag, CONCAT(test_func_, __LINE__)); \
    static void CONCAT(test_func_, __LINE__)()

#define REQUIRE(expr) \
    do { if (!(expr)) { \
        std::cerr << "  FAILED: " << #expr << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        throw std::runtime_error("REQUIRE failed"); \
    } } while(0)

#ifdef CATCH_CONFIG_MAIN
int main() {
    int passed = 0, failed = 0;
    for (auto& tc : test_registry()) {
        std::cout << "[" << tc.tag << "] " << tc.name << " ... ";
        try {
            tc.func();
            std::cout << "PASSED" << std::endl;
            passed++;
        } catch (const std::exception& e) {
            std::cout << "FAILED (" << e.what() << ")" << std::endl;
            failed++;
        }
    }
    std::cout << "\n" << passed << " passed, " << failed << " failed" << std::endl;
    return failed > 0 ? 1 : 0;
}
#endif
