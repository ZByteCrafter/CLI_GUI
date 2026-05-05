#include "catch2.hpp"
#include <CLI_GUI/CLI_GUI.hpp>

TEST_CASE("CLI_GUI::App can be default-constructed", "[app]") {
    CLI_GUI::App app{"Description", "Test App"};
    REQUIRE(app.get_name() == "Test App");
}

TEST_CASE("CLI_GUI::App inherits CLI::App API", "[app]") {
    CLI_GUI::App app{"Test"};
    int val = 0;
    auto* opt = app.add_option("-n,--count", val, "A number");
    REQUIRE(opt != nullptr);
    REQUIRE(opt->get_name() == "--count");
}

TEST_CASE("CLI_GUI_PARSE parses arguments in CLI mode", "[app][parse]") {
    CLI_GUI::App app{"Test"};
    int val = 0;
    app.add_option("-n", val, "number");
    const char* argv[] = {"test", "-n", "42"};
    CLI_GUI_PARSE(app, 3, const_cast<char**>(argv));
    REQUIRE(val == 42);
}
