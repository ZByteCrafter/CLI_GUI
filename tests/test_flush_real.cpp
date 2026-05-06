#ifdef CLI_GUI_HAS_GUI

#include "catch2.hpp"
#include <CLI_GUI/CLI_GUI.hpp>
#include <CLI_GUI/GuiLauncher.hpp>
#include <cstring>

// Tests for the actual flush_gui_to_cli function (not manual argv simulation).

TEST_CASE("flush_gui_to_cli: text option flushed via real function", "[flush_real]") {
    CLI_GUI::App app{"Test"};
    std::string name;
    app.add_option("-n,--name", name, "name");
    auto& meta = app.gui_meta(app.get_option("--name"));
    std::strcpy(meta.text_buf, "Alice");
    meta.initialized = true;

    CLI_GUI::flush_gui_to_cli(app);
    REQUIRE(name == "Alice");
}

TEST_CASE("flush_gui_to_cli: bool flag flushed via real function", "[flush_real]") {
    CLI_GUI::App app{"Test"};
    bool verbose = false;
    app.add_flag("-v,--verbose", verbose, "verbose");
    auto& meta = app.gui_meta(app.get_option("--verbose"));
    meta.bool_state = true;

    CLI_GUI::flush_gui_to_cli(app);
    REQUIRE(verbose == true);
}

TEST_CASE("flush_gui_to_cli: int option flushed via real function", "[flush_real]") {
    CLI_GUI::App app{"Test"};
    int count = 0;
    app.add_option("-c,--count", count, "count");
    auto& meta = app.gui_meta(app.get_option("--count"));
    meta.int_state = 42;
    meta.initialized = true;

    CLI_GUI::flush_gui_to_cli(app);
    REQUIRE(count == 42);
}

TEST_CASE("flush_gui_to_cli: float option flushed via real function", "[flush_real]") {
    CLI_GUI::App app{"Test"};
    float ratio = 0.0f;
    app.add_option("-r,--ratio", ratio, "ratio");
    auto& meta = app.gui_meta(app.get_option("--ratio"));
    meta.float_state = 0.75f;
    meta.initialized = true;

    CLI_GUI::flush_gui_to_cli(app);
    REQUIRE(ratio == 0.75f);
}

TEST_CASE("flush_gui_to_cli: combo option flushed via real function", "[flush_real]") {
    CLI_GUI::App app{"Test"};
    std::string mode;
    auto* opt = app.add_option("-m,--mode", mode, "mode");
    CLI_GUI::gui_values(opt, {"fast", "normal", "slow"}, app);
    auto& meta = app.gui_meta(opt);
    meta.combo_current = 2;
    meta.initialized = true;

    CLI_GUI::flush_gui_to_cli(app);
    REQUIRE(mode == "slow");
}

TEST_CASE("flush_gui_to_cli: list items flushed via real function", "[flush_real]") {
    CLI_GUI::App app{"Test"};
    std::vector<std::string> files;
    auto* opt = app.add_option("-f,--file", files, "files");
    // Must explicitly set List widget type for vector options
    CLI_GUI::gui_widget(opt, CLI_GUI::WidgetType::List, app);
    auto& meta = app.gui_meta(opt);
    meta.list_items = {"a.txt", "b.txt"};
    meta.initialized = true;

    CLI_GUI::flush_gui_to_cli(app);
    REQUIRE(files.size() == 2);
    REQUIRE(files[0] == "a.txt");
    REQUIRE(files[1] == "b.txt");
}

TEST_CASE("flush_gui_to_cli: subcommand option flushed via real function", "[flush_real]") {
    CLI_GUI::App app{"Root"};
    auto* sub = app.add_subcommand("cmd", "sub");
    std::string output;
    sub->add_option("-o,--output", output, "output");
    auto& meta = app.gui_meta(sub->get_option("--output"));
    std::strcpy(meta.text_buf, "out.txt");
    meta.initialized = true;

    CLI_GUI::flush_gui_to_cli(app, "cmd");
    REQUIRE(sub->parsed());
    REQUIRE(output == "out.txt");
}

TEST_CASE("flush_gui_to_cli: uninitialized int option not flushed", "[flush_real]") {
    CLI_GUI::App app{"Test"};
    int count = 10;
    app.add_option("-c,--count", count, "count");
    // Don't set initialized = true
    auto& meta = app.gui_meta(app.get_option("--count"));
    meta.int_state = 0;

    CLI_GUI::flush_gui_to_cli(app);
    REQUIRE(count == 10);  // default preserved
}

#endif // CLI_GUI_HAS_GUI
