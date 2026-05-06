#include "catch2.hpp"
#include <CLI_GUI/CLI_GUI.hpp>
#include <cstring>
#include <vector>

// Tests for the flush_gui_to_cli data path:
// Build fake argv from OptionGuiMeta state, call app.parse(), verify variables.

TEST_CASE("flush: text_buf written to variable via parse", "[flush]") {
    CLI_GUI::App app{"Test"};
    std::string name;
    app.add_option("-n,--name", name, "name");
    auto& meta = app.gui_meta(app.get_option("--name"));
    std::strcpy(meta.text_buf, "John");
    meta.initialized = true;

    // Simulate flush: build fake argv
    const char* argv[] = {"test", "--name", meta.text_buf};
    app.parse(3, const_cast<char**>(argv));
    REQUIRE(name == "John");
}

TEST_CASE("flush: bool_state checked = flag present", "[flush]") {
    CLI_GUI::App app{"Test"};
    bool flag = false;
    app.add_flag("-v,--verbose", flag, "verbose");
    auto& meta = app.gui_meta(app.get_option("--verbose"));
    meta.bool_state = true;

    // flag true → include "--verbose"
    std::vector<const char*> argv = {"test", "--verbose"};
    app.parse(2, const_cast<char**>(argv.data()));
    REQUIRE(flag == true);
}

TEST_CASE("flush: bool_state unchecked = flag absent", "[flush]") {
    CLI_GUI::App app{"Test"};
    bool flag = false;
    app.add_flag("-v,--verbose", flag, "verbose");
    auto& meta = app.gui_meta(app.get_option("--verbose"));
    meta.bool_state = false;

    // flag false → don't include "--verbose"
    std::vector<const char*> argv = {"test"};
    app.parse(1, const_cast<char**>(argv.data()));
    REQUIRE(flag == false);
}

TEST_CASE("flush: int_state written to variable", "[flush]") {
    CLI_GUI::App app{"Test"};
    int count = 0;
    app.add_option("-c,--count", count, "count");
    auto& meta = app.gui_meta(app.get_option("--count"));
    meta.int_state = 42;

    std::string val = std::to_string(meta.int_state);
    const char* argv[] = {"test", "--count", val.c_str()};
    app.parse(3, const_cast<char**>(argv));
    REQUIRE(count == 42);
}

TEST_CASE("flush: slider float written to variable", "[flush]") {
    CLI_GUI::App app{"Test"};
    float ratio = 0.0f;
    app.add_option("-r,--ratio", ratio, "ratio");
    auto& meta = app.gui_meta(app.get_option("--ratio"));
    meta.float_state = 0.75f;

    std::string val = std::to_string(meta.float_state);
    const char* argv[] = {"test", "--ratio", val.c_str()};
    app.parse(3, const_cast<char**>(argv));
    REQUIRE(ratio == 0.75f);
}

TEST_CASE("flush: combo selected value written", "[flush]") {
    CLI_GUI::App app{"Test"};
    std::string mode;
    auto* opt = app.add_option("-m,--mode", mode, "mode");
    CLI_GUI::gui_values(opt, {"fast", "normal", "slow"}, app);
    auto& meta = app.gui_meta(opt);
    meta.combo_current = 2;  // "slow"

    const char* argv[] = {"test", "--mode", "slow"};
    app.parse(3, const_cast<char**>(argv));
    REQUIRE(mode == "slow");
}

TEST_CASE("flush: list items repeated for each value", "[flush]") {
    CLI_GUI::App app{"Test"};
    std::vector<std::string> files;
    app.add_option("-f,--file", files, "files");

    auto& meta = app.gui_meta(app.get_option("--file"));
    meta.list_items = {"a.txt", "b.txt", "c.txt"};

    const char* argv[] = {"test", "--file", "a.txt", "--file", "b.txt", "--file", "c.txt"};
    app.parse(7, const_cast<char**>(argv));
    REQUIRE(files.size() == 3);
    REQUIRE(files[0] == "a.txt");
    REQUIRE(files[2] == "c.txt");
}

TEST_CASE("flush: subcommand option values written", "[flush]") {
    CLI_GUI::App app{"Root"};
    auto* sub = app.add_subcommand("cmd", "sub");
    std::string val;
    sub->add_option("-o,--output", val, "output");
    auto& meta = app.gui_meta(sub->get_option("--output"));
    std::strcpy(meta.text_buf, "result.txt");
    meta.initialized = true;

    // Simulate: root options + subcommand name + subcommand options
    const char* argv[] = {"test", "cmd", "--output", meta.text_buf};
    app.parse(4, const_cast<char**>(argv));
    REQUIRE(sub->parsed());
    REQUIRE(val == "result.txt");
}
