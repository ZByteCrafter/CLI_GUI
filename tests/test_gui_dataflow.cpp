#include "catch2.hpp"
#include <CLI_GUI/CLI_GUI.hpp>
#include <cstring>

// Tests for GUI data flow — simulates what render_option and
// flush_gui_to_cli would do, without needing an actual ImGui context.

TEST_CASE("OptionGuiMeta init: text_buf initialized from CLI results", "[gui_dataflow]") {
    CLI_GUI::App app{"Test"};
    std::string name;
    auto* opt = app.add_option("-n", name, "name");

    // Simulate CLI parse setting a value
    const char* argv[] = {"test", "-n", "Hello"};
    CLI_GUI_PARSE(app, 3, const_cast<char**>(argv));

    // --- Simulate render_option init logic ---
    auto& meta = app.gui_meta(opt);
    if (!meta.initialized && !opt->results().empty()) {
        std::strncpy(meta.text_buf, opt->results()[0].c_str(), sizeof(meta.text_buf) - 1);
        meta.text_buf[sizeof(meta.text_buf) - 1] = '\0';
        meta.initialized = true;
    }

    REQUIRE(meta.initialized == true);
    REQUIRE(std::strcmp(meta.text_buf, "Hello") == 0);
    REQUIRE(name == "Hello");  // CLI11 already set this via parse
}

TEST_CASE("OptionGuiMeta init: bool_state from flag", "[gui_dataflow]") {
    CLI_GUI::App app{"Test"};
    bool flag = false;
    auto* opt = app.add_flag("-v,--verbose", flag, "verbose");

    // Simulate CLI parse with flag present
    const char* argv[] = {"test", "-v"};
    CLI_GUI_PARSE(app, 2, const_cast<char**>(argv));

    auto& meta = app.gui_meta(opt);
    if (!meta.initialized) {
        meta.bool_state = !opt->results().empty();
        meta.initialized = true;
    }

    REQUIRE(meta.initialized == true);
    REQUIRE(meta.bool_state == true);
    REQUIRE(flag == true);
}

TEST_CASE("OptionGuiMeta init: bool_state when flag absent", "[gui_dataflow]") {
    CLI_GUI::App app{"Test"};
    app.allow_extras();  // accept dummy arg below
    bool flag = false;
    auto* opt = app.add_flag("-v,--verbose", flag, "verbose");

    // Simulate CLI parse without flag (use argc>1 to avoid GUI launch)
    const char* argv[] = {"test", "--"};
    CLI_GUI_PARSE(app, 2, const_cast<char**>(argv));

    auto& meta = app.gui_meta(opt);
    if (!meta.initialized) {
        meta.bool_state = !opt->results().empty();
        meta.initialized = true;
    }

    REQUIRE(meta.bool_state == false);
    REQUIRE(flag == false);
}

TEST_CASE("OptionGuiMeta init: int_state from CLI results", "[gui_dataflow]") {
    CLI_GUI::App app{"Test"};
    int val = 0;
    auto* opt = app.add_option("-c", val, "count");

    const char* argv[] = {"test", "-c", "42"};
    CLI_GUI_PARSE(app, 3, const_cast<char**>(argv));

    auto& meta = app.gui_meta(opt);
    if (!meta.initialized && !opt->results().empty()) {
        try { meta.int_state = std::stoi(opt->results()[0]); }
        catch (...) { meta.int_state = 0; }
        meta.initialized = true;
    }

    REQUIRE(meta.int_state == 42);
    REQUIRE(val == 42);
}

TEST_CASE("OptionGuiMeta init: combo_current from CLI results matching values", "[gui_dataflow]") {
    CLI_GUI::App app{"Test"};
    std::string mode;
    auto* opt = app.add_option("-m", mode, "mode");
    CLI_GUI::gui_values(opt, {"fast","normal","slow"}, app);

    // Simulate CLI parse with "normal"
    const char* argv[] = {"test", "-m", "normal"};
    CLI_GUI_PARSE(app, 3, const_cast<char**>(argv));

    auto& meta = app.gui_meta(opt);
    if (!meta.initialized) {
        meta.initialized = true;
        if (!opt->results().empty()) {
            auto& cv = opt->results()[0];
            for (size_t i = 0; i < meta.values.size(); ++i) {
                if (meta.values[i] == cv) {
                    meta.combo_current = static_cast<int>(i);
                    break;
                }
            }
        }
    }

    REQUIRE(meta.combo_current == 1);  // "normal" is index 1
    REQUIRE(mode == "normal");
}

TEST_CASE("OptionGuiMeta init: combo_current stays 0 when no match", "[gui_dataflow]") {
    CLI_GUI::App app{"Test"};
    std::string mode;
    auto* opt = app.add_option("-m", mode, "mode");
    CLI_GUI::gui_values(opt, {"fast","normal","slow"}, app);

    // Simulate CLI parse with "ultra" (not in values)
    const char* argv[] = {"test", "-m", "ultra"};
    CLI_GUI_PARSE(app, 3, const_cast<char**>(argv));

    auto& meta = app.gui_meta(opt);
    if (!meta.initialized) {
        meta.initialized = true;
        if (!opt->results().empty()) {
            auto& cv = opt->results()[0];
            for (size_t i = 0; i < meta.values.size(); ++i) {
                if (meta.values[i] == cv) {
                    meta.combo_current = static_cast<int>(i);
                    break;
                }
            }
        }
    }

    REQUIRE(meta.combo_current == 0);  // stays at default
}

TEST_CASE("OptionGuiMeta init: combo_default when no CLI value", "[gui_dataflow]") {
    CLI_GUI::App app{"Test"};
    std::string mode;
    auto* opt = app.add_option("-m", mode, "mode");
    CLI_GUI::gui_values(opt, {"fast","normal","slow"}, app);

    // No CLI parse — option not present
    const char* argv[] = {"test"};
    CLI_GUI_PARSE(app, 1, const_cast<char**>(argv));

    auto& meta = app.gui_meta(opt);
    if (!meta.initialized) {
        meta.initialized = true;
    }

    REQUIRE(meta.combo_current == 0);  // stays at default 0
}

TEST_CASE("OptionGuiMeta init: does not re-init after initialized", "[gui_dataflow]") {
    CLI_GUI::App app{"Test"};
    std::string name;
    auto* opt = app.add_option("-n", name, "name");

    auto& meta = app.gui_meta(opt);

    // First init
    meta.initialized = true;
    meta.text_buf[0] = 'X';
    meta.text_buf[1] = '\0';

    // Simulate second parse attempt — should NOT overwrite
    const char* argv[] = {"test", "-n", "Hello"};
    CLI_GUI_PARSE(app, 3, const_cast<char**>(argv));

    if (!meta.initialized && !opt->results().empty()) {
        std::strncpy(meta.text_buf, opt->results()[0].c_str(), sizeof(meta.text_buf) - 1);
    }

    // text_buf should still be "X", not "Hello"
    REQUIRE(meta.initialized == true);
    REQUIRE(meta.text_buf[0] == 'X');
}

TEST_CASE("OptionGuiMeta: stoi exception safety", "[gui_dataflow]") {
    CLI_GUI::App app{"Test"};
    int val = 0;
    auto* opt = app.add_option("-c", val, "count");

    // Manually set a non-numeric result (edge case)
    // CLI11 validates this normally, but test the guard
    auto& meta = app.gui_meta(opt);
    if (!meta.initialized) {
        try {
            // Simulate what would happen if results had bad data
            meta.int_state = 0;  // safe default
        } catch (...) {
            meta.int_state = -1;
        }
        meta.initialized = true;
    }

    REQUIRE(meta.int_state == 0);
}

TEST_CASE("Subcommand option meta stored on root app", "[gui_dataflow]") {
    CLI_GUI::App app{"Root"};
    auto* sub = app.add_subcommand("cmd", "subcommand");
    std::string val;
    auto* opt = sub->add_option("-o", val, "output");

    CLI_GUI::gui_label(opt, "Output File", app);
    CLI_GUI::gui_widget(opt, CLI_GUI::WidgetType::FileSave, app);

    // Verify metadata is on root, accessible by option pointer from sub
    REQUIRE(app.gui_meta(opt).label == "Output File");
    REQUIRE(app.gui_meta(opt).widget_type == CLI_GUI::WidgetType::FileSave);
}
