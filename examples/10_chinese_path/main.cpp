// Example 10: Chinese path encoding test
// GUI mode: all strings are UTF-8 (ImGui native).
// Use CLI_GUI::utf8_to_ansi() when you need GBK for Win32 APIs or std::filesystem.
// Use CLI_GUI::ansi_to_utf8() when you have a GBK string and need UTF-8 for GUI display.
#include <CLI_GUI/CLI_GUI.hpp>
#include <CLI_GUI/detail/Win32SuppressConsole.hpp>
#include <iostream>
#include <filesystem>

int main(int argc, char** argv) {
    cli_gui_init_console();
    CLI_GUI::App app{"Chinese Path Test"};
    app.gui_size(600, 400);
    app.gui_encoding(CLI_GUI::Encoding::Utf8);

    std::string dir_path = ".";
    app.add_option("-d,--dir", dir_path, "Directory path (UTF-8 in GUI mode)");
    CLI_GUI::gui_label(app.get_option("--dir"), "Folder", app);
    CLI_GUI::gui_widget(app.get_option("--dir"), CLI_GUI::WidgetType::DirPicker, app);

    std::string file_path;
    app.add_option("-f,--file", file_path, "File path (UTF-8 in GUI mode)");
    CLI_GUI::gui_label(app.get_option("--file"), "File", app);
    CLI_GUI::gui_widget(app.get_option("--file"), CLI_GUI::WidgetType::FileOpen, app);

    app.set_main([&]() {
        std::cout << "=== Chinese Path Test ===" << std::endl;
        std::cout << "Directory: " << dir_path << std::endl;
        // std::filesystem on Windows expects GBK, so convert from callback encoding
        std::string fs_dir = CLI_GUI::utf8_to_ansi(dir_path);
        bool dir_ok = std::filesystem::exists(fs_dir);
        std::cout << "  exists: " << (dir_ok ? "YES" : "NO") << std::endl;

        if (!file_path.empty()) {
            std::cout << "File:      " << file_path << std::endl;
            std::string fs_file = CLI_GUI::utf8_to_ansi(file_path);
            bool file_ok = std::filesystem::exists(fs_file);
            std::cout << "  exists: " << (file_ok ? "YES" : "NO") << std::endl;
        }
    });

    CLI_GUI_PARSE(app, argc, argv);
    return 0;
}
