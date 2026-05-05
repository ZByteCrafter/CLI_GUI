#include <CLI_GUI/CLI_GUI.hpp>
#include <CLI_GUI/detail/Win32SuppressConsole.hpp>
#include <iostream>

int main(int argc, char** argv) {
    cli_gui_init_console();
    CLI_GUI::App app{"Data Tool"};
    app.gui_size(900, 650);

    std::string input = "data.txt";
    app.add_option("-i,--input", input, "Input file");
    CLI_GUI::gui_label(app.get_option("--input"), "Input File", app);

    auto* analyze = app.add_subcommand("analyze", "Analyze the data");
    std::string method = "fast";
    analyze->add_option("-m,--method", method, "Analysis method");
    CLI_GUI::gui_values(analyze->get_option("--method"), {"fast","precise","balanced"}, app);

    auto* export_cmd = app.add_subcommand("export", "Export results");
    std::string format = "json";
    export_cmd->add_option("-f,--format", format, "Output format");
    CLI_GUI::gui_values(export_cmd->get_option("--format"), {"json","csv","xml","txt"}, app);

    app.set_main([&]() {
        if (analyze->parsed()) {
            std::cout << "Analyzing " << input << " with method=" << method << std::endl;
        } else if (export_cmd->parsed()) {
            std::cout << "Exporting " << input << " as " << format << std::endl;
        } else {
            std::cout << "Processing " << input << std::endl;
        }
    });

    CLI_GUI_PARSE(app, argc, argv);
    return 0;
}
