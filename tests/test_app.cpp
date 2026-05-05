#define CATCH_CONFIG_MAIN
#include "catch2.hpp"
#include <CLI_GUI/CLI_GUI.hpp>

TEST_CASE("CLI_GUI::App can be constructed with description", "[app]") {
    CLI_GUI::App app{"My Description", "Test App"};
    REQUIRE(app.get_name() == "Test App");
    REQUIRE(app.get_description() == "My Description");
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

TEST_CASE("Option stores gui_label via free function", "[app][gui_meta]") {
    CLI_GUI::App app{"Test"};
    int val = 0;
    auto* opt = app.add_option("-n", val, "count");
    CLI_GUI::gui_label(opt, "Number", app);
    REQUIRE(app.gui_meta(opt).label == "Number");
}

TEST_CASE("Option stores gui_widget override", "[app][gui_meta]") {
    CLI_GUI::App app{"Test"};
    int val = 0;
    auto* opt = app.add_option("-n", val, "count");
    CLI_GUI::gui_widget(opt, CLI_GUI::WidgetType::SliderInt, app);
    REQUIRE(app.gui_meta(opt).widget_type == CLI_GUI::WidgetType::SliderInt);
}

TEST_CASE("Option without override has WidgetType::Auto", "[app][gui_meta]") {
    CLI_GUI::App app{"Test"};
    int val = 0;
    auto* opt = app.add_option("-n", val, "count");
    REQUIRE(app.gui_meta(opt).widget_type == CLI_GUI::WidgetType::Auto);
}

TEST_CASE("Option stores gui_min and gui_max", "[app][gui_meta]") {
    CLI_GUI::App app{"Test"};
    int val = 0;
    auto* opt = app.add_option("-n", val, "count");
    CLI_GUI::gui_min(opt, 0.0, app);
    CLI_GUI::gui_max(opt, 100.0, app);
    REQUIRE(app.gui_meta(opt).has_min == true);
    REQUIRE(app.gui_meta(opt).min_val == 0.0);
    REQUIRE(app.gui_meta(opt).max_val == 100.0);
}

TEST_CASE("Option stores gui_values", "[app][gui_meta]") {
    CLI_GUI::App app{"Test"};
    std::string mode;
    auto* opt = app.add_option("-m", mode, "mode");
    CLI_GUI::gui_values(opt, {"fast","normal","slow"}, app);
    REQUIRE(app.gui_meta(opt).values.size() == 3);
    REQUIRE(app.gui_meta(opt).values[0] == "fast");
}

TEST_CASE("App stores global GUI settings", "[app][gui_meta]") {
    CLI_GUI::App app{"MyTool"};
    app.gui_title("Custom Title");
    app.gui_size(1024, 768);
    app.gui_show_console(false);
    app.gui_console_height(200);
    REQUIRE(app.gui_title() == "Custom Title");
    REQUIRE(app.gui_width() == 1024);
    REQUIRE(app.gui_height() == 768);
    REQUIRE(app.gui_show_console() == false);
    REQUIRE(app.gui_console_height() == 200);
}

TEST_CASE("App default title is app name", "[app][gui_meta]") {
    CLI_GUI::App app{"Descr", "MyApp"};
    REQUIRE(app.gui_title() == "MyApp");
}

TEST_CASE("App set_callback stores function", "[app][gui_meta]") {
    CLI_GUI::App app{"Test"};
    bool called = false;
    app.set_callback([&called]() { called = true; });
    auto cb = app.gui_callback();
    REQUIRE(cb != nullptr);
    cb();
    REQUIRE(called == true);
}

TEST_CASE("App update_progress and get_progress work", "[app][gui_meta]") {
    CLI_GUI::App app{"Test"};
    REQUIRE(app.get_progress() == 0.0f);
    app.update_progress(0.75f);
    REQUIRE(app.get_progress() == 0.75f);
}

TEST_CASE("App cancel flag works", "[app][gui_meta]") {
    CLI_GUI::App app{"Test"};
    REQUIRE(app.is_cancelled() == false);
    app.request_cancel();
    REQUIRE(app.is_cancelled() == true);
    app.reset_cancel();
    REQUIRE(app.is_cancelled() == false);
}
