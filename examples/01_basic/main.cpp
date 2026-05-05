#include <CLI_GUI/CLI_GUI.hpp>
#include <CLI_GUI/detail/Win32SuppressConsole.hpp>
#include <iostream>
#include <cctype>

int main(int argc, char** argv) {
    cli_gui_init_console();
    CLI_GUI::App app{"Hello GUI"};

    std::string name = "World";
    app.add_option("-n,--name", name, "Your name");
    CLI_GUI::gui_label(app.get_option("--name"), "Your Name", app);

    int count = 1;
    app.add_option("-c,--count", count, "Repeat count");
    CLI_GUI::gui_widget(app.get_option("--count"), CLI_GUI::WidgetType::SliderInt, app);
    CLI_GUI::gui_min(app.get_option("--count"), 1, app);
    CLI_GUI::gui_max(app.get_option("--count"), 20, app);

    bool uppercase = false;
    app.add_flag("-u,--upper", uppercase, "Uppercase output");

    app.set_main([&]() {
        for (int i = 0; i < count; ++i) {
            std::string msg = "Hello, " + name + "!";
            if (uppercase) {
                for (auto& c : msg)
                    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            }
            std::cout << "[" << (i + 1) << "] " << msg << std::endl;
        }
    });

    CLI_GUI_PARSE(app, argc, argv);
    return 0;
}
