// Example 06: Color pickers + file dialogs + drag-drop
// Demonstrates ColorRGB, ColorRGBA, FileOpen, FileSave, DirPicker, FileOrDir.
// Tip: drag files from Explorer onto any file text field to fill the path.
#include <CLI_GUI/CLI_GUI.hpp>
#include <CLI_GUI/detail/Win32SuppressConsole.hpp>
#include <iostream>

int main(int argc, char** argv) {
    cli_gui_init_console();
    CLI_GUI::App app{"Theme Designer"};
    app.gui_size(850, 650);

    // ── Colors ──
    std::string bg_color = "1.0 1.0 1.0";
    app.add_option("--bg", bg_color, "Background color (R G B)");
    CLI_GUI::gui_label(app.get_option("--bg"), "Background", app);
    CLI_GUI::gui_widget(app.get_option("--bg"), CLI_GUI::WidgetType::ColorRGB, app);

    std::string fg_color = "0.0 0.0 0.0";
    app.add_option("--fg", fg_color, "Foreground color (R G B)");
    CLI_GUI::gui_label(app.get_option("--fg"), "Foreground", app);
    CLI_GUI::gui_widget(app.get_option("--fg"), CLI_GUI::WidgetType::ColorRGB, app);

    std::string accent = "0.2 0.5 1.0 1.0";
    app.add_option("--accent", accent, "Accent color (R G B A)");
    CLI_GUI::gui_label(app.get_option("--accent"), "Accent", app);
    CLI_GUI::gui_widget(app.get_option("--accent"), CLI_GUI::WidgetType::ColorRGBA, app);

    // ── File paths ──
    std::string config_file;
    app.add_option("-c,--config", config_file, "Config file to load");
    CLI_GUI::gui_label(app.get_option("--config"), "Config File", app);
    CLI_GUI::gui_widget(app.get_option("--config"), CLI_GUI::WidgetType::FileOpen, app);

    std::string output_dir = ".";
    app.add_option("-o,--output", output_dir, "Output directory");
    CLI_GUI::gui_label(app.get_option("--output"), "Output Dir", app);
    CLI_GUI::gui_widget(app.get_option("--output"), CLI_GUI::WidgetType::DirPicker, app);

    std::string export_path = "theme.json";
    app.add_option("--export", export_path, "Export theme to");
    CLI_GUI::gui_label(app.get_option("--export"), "Export As", app);
    CLI_GUI::gui_widget(app.get_option("--export"), CLI_GUI::WidgetType::FileSave, app);

    // ── Composite file/folder picker ──
    std::string project_path = "./project";
    app.add_option("--project", project_path, "Project path (file or folder)");
    CLI_GUI::gui_label(app.get_option("--project"), "Project", app);
    CLI_GUI::gui_widget(app.get_option("--project"), CLI_GUI::WidgetType::FileOrDir, app);

    // ── Text options ──
    std::string api_key;
    app.add_option("-k,--key", api_key, "API key");
    CLI_GUI::gui_label(app.get_option("--key"), "API Key", app);
    CLI_GUI::gui_widget(app.get_option("--key"), CLI_GUI::WidgetType::Password, app);

    app.set_main([&]() {
        std::cout << "Background:  " << bg_color << std::endl;
        std::cout << "Foreground:  " << fg_color << std::endl;
        std::cout << "Accent:      " << accent << std::endl;
        std::cout << "Config:      " << config_file << std::endl;
        std::cout << "Output dir:  " << output_dir << std::endl;
        std::cout << "Export:      " << export_path << std::endl;
        std::cout << "Project:     " << project_path << std::endl;
    });

    CLI_GUI_PARSE(app, argc, argv);
    return 0;
}
