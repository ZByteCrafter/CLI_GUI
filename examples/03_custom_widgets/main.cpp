#include <CLI_GUI/CLI_GUI.hpp>
#include <CLI_GUI/detail/Win32SuppressConsole.hpp>
#include <iostream>

int main(int argc, char** argv) {
    cli_gui_init_console();
    CLI_GUI::App app{"Custom Widgets Demo"};
    app.gui_size(850, 700);

    std::string password;
    app.add_option("-p,--password", password, "API key");
    CLI_GUI::gui_label(app.get_option("--password"), "Secret Key", app);
    CLI_GUI::gui_widget(app.get_option("--password"), CLI_GUI::WidgetType::Password, app);

    std::string mode;
    app.add_option("-m,--mode", mode, "Operation mode");
    CLI_GUI::gui_widget(app.get_option("--mode"), CLI_GUI::WidgetType::Radio, app);
    CLI_GUI::gui_values(app.get_option("--mode"), {"fast","normal","thorough"}, app);

    std::string config;
    app.add_option("-c,--config", config, "JSON config");
    CLI_GUI::gui_label(app.get_option("--config"), "Config (JSON)", app);
    CLI_GUI::gui_widget(app.get_option("--config"), CLI_GUI::WidgetType::CodeEditor, app);

    std::string output;
    app.add_option("-o,--output", output, "Save to file");
    CLI_GUI::gui_widget(app.get_option("--output"), CLI_GUI::WidgetType::FileSave, app);

    app.set_main([&]() {
        std::cout << "Mode: " << mode << std::endl;
        std::cout << "Config: " << config.substr(0, 50)
                  << (config.size() > 50 ? "..." : "") << std::endl;
        std::cout << "Output: " << output << std::endl;
    });

    CLI_GUI_PARSE(app, argc, argv);
    return 0;
}
