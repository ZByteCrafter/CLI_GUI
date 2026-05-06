// Example 07: Positional arguments + vector options
// Demonstrates positional arguments and multi-value (vector) options.
#include <CLI_GUI/CLI_GUI.hpp>
#include <CLI_GUI/detail/Win32SuppressConsole.hpp>
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    cli_gui_init_console();
    CLI_GUI::App app{"Batch Renamer"};
    app.gui_size(900, 700);

    // ── Positional argument: files ──
    std::vector<std::string> files;
    app.add_option("files", files, "Files to rename")
       ->required();
    CLI_GUI::gui_label(app.get_option("files"), "Files", app);

    // ── Prefix / suffix ──
    std::string prefix;
    app.add_option("--prefix", prefix, "Prefix to add");
    CLI_GUI::gui_label(app.get_option("--prefix"), "Prefix", app);

    std::string suffix;
    app.add_option("--suffix", suffix, "Suffix to add");
    CLI_GUI::gui_label(app.get_option("--suffix"), "Suffix", app);

    // ── Include / exclude patterns (vectors) ──
    std::vector<std::string> includes;
    app.add_option("-I,--include", includes, "Include pattern (repeatable)");
    CLI_GUI::gui_label(app.get_option("--include"), "Include Patterns", app);

    std::vector<std::string> excludes;
    app.add_option("-E,--exclude", excludes, "Exclude pattern (repeatable)");
    CLI_GUI::gui_label(app.get_option("--exclude"), "Exclude Patterns", app);

    // ── Flags ──
    bool dry_run = false;
    app.add_flag("--dry-run", dry_run, "Preview only (no changes)");

    bool recursive = false;
    app.add_flag("-r,--recursive", recursive, "Process subdirectories");

    int timeout = 30;
    app.add_option("--timeout", timeout, "Timeout");
    CLI_GUI::gui_widget(app.get_option("--timeout"), CLI_GUI::WidgetType::Duration, app);

    app.set_main([&]() {
        std::cout << "Files to rename: " << files.size() << std::endl;
        for (auto& f : files) std::cout << "  " << f << std::endl;

        if (!prefix.empty()) std::cout << "Prefix: " << prefix << std::endl;
        if (!suffix.empty()) std::cout << "Suffix: " << suffix << std::endl;

        if (!includes.empty()) {
            std::cout << "Include patterns:" << std::endl;
            for (auto& p : includes) std::cout << "  " << p << std::endl;
        }
        if (!excludes.empty()) {
            std::cout << "Exclude patterns:" << std::endl;
            for (auto& p : excludes) std::cout << "  " << p << std::endl;
        }

        std::cout << "Dry-run: " << (dry_run ? "yes" : "no") << std::endl;
        std::cout << "Recursive: " << (recursive ? "yes" : "no") << std::endl;
        std::cout << "Timeout: " << timeout << " seconds" << std::endl;
    });

    CLI_GUI_PARSE(app, argc, argv);
    return 0;
}
