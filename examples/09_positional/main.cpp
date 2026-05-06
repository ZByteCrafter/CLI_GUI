// Example 09: Positional arguments
// Demonstrates positional arguments (no - or -- prefix).
// Run CLI: 09_positional.exe alice bob --greeting "Hi" --count 3
// Run GUI: double-click and fill the form.
#include <CLI_GUI/CLI_GUI.hpp>
#include <CLI_GUI/detail/Win32SuppressConsole.hpp>
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    cli_gui_init_console();
    CLI_GUI::App app{"Greeter"};
    app.gui_size(800, 600);

    // ── Positional arguments (no - or -- prefix) ──
    std::vector<std::string> names;
    app.add_option("names", names, "Names to greet")
       ->required()
       ->expected(1, 10);  // 1 to 10 names
    CLI_GUI::gui_label(app.get_option("names"), "Names", app);

    // ── Regular options ──
    std::string greeting = "Hello";
    app.add_option("-g,--greeting", greeting, "Greeting word");
    CLI_GUI::gui_label(app.get_option("--greeting"), "Greeting", app);

    int count = 1;
    app.add_option("-c,--count", count, "Repeat count");
    CLI_GUI::gui_widget(app.get_option("--count"), CLI_GUI::WidgetType::SliderInt, app);
    CLI_GUI::gui_min(app.get_option("--count"), 1, app);
    CLI_GUI::gui_max(app.get_option("--count"), 10, app);

    bool uppercase = false;
    app.add_flag("-u,--upper", uppercase, "UPPERCASE output");

    // ── Main logic ──
    app.set_main([&]() {
        std::cout << "Greeting: " << greeting << std::endl;
        std::cout << "Repeat:   " << count << " time(s)" << std::endl;
        std::cout << "Uppercase: " << (uppercase ? "yes" : "no") << std::endl;
        std::cout << "Names (" << names.size() << "):" << std::endl;
        for (int r = 0; r < count; ++r) {
            for (auto& n : names) {
                std::string msg = greeting + ", " + n + "!";
                if (uppercase)
                    for (auto& c : msg) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
                std::cout << "  " << msg << std::endl;
            }
        }
    });

    CLI_GUI_PARSE(app, argc, argv);
    return 0;
}
