// Example 05: CLI::OptionGroup -> collapsible panels in GUI
// Demonstrates how option groups are rendered as foldable sections.
#include <CLI_GUI/CLI_GUI.hpp>
#include <iostream>

int main(int argc, char** argv) {
    CLI_GUI::App app{"Image Processor v2.0"};
    app.gui_size(900, 700);

    // ── Group: Input ──
    auto* input_group = app.add_option_group("Input");

    std::string source = "image.png";
    input_group->add_option("-s,--source", source, "Source image");
    CLI_GUI::gui_label(input_group->get_option("--source"), "Source File", app);
    CLI_GUI::gui_widget(input_group->get_option("--source"),
                        CLI_GUI::WidgetType::FileOpen, app);

    int width = 1920;
    input_group->add_option("-W,--width", width, "Target width (px)");
    CLI_GUI::gui_widget(input_group->get_option("--width"),
                        CLI_GUI::WidgetType::SliderInt, app);
    CLI_GUI::gui_min(input_group->get_option("--width"), 1, app);
    CLI_GUI::gui_max(input_group->get_option("--width"), 7680, app);

    int height = 1080;
    input_group->add_option("-H,--height", height, "Target height (px)");
    CLI_GUI::gui_widget(input_group->get_option("--height"),
                        CLI_GUI::WidgetType::SliderInt, app);
    CLI_GUI::gui_min(input_group->get_option("--height"), 1, app);
    CLI_GUI::gui_max(input_group->get_option("--height"), 4320, app);

    // ── Group: Output ──
    auto* output_group = app.add_option_group("Output");

    std::string dest = "output.png";
    output_group->add_option("-d,--dest", dest, "Output path");
    CLI_GUI::gui_label(output_group->get_option("--dest"), "Save To", app);
    CLI_GUI::gui_widget(output_group->get_option("--dest"),
                        CLI_GUI::WidgetType::FileSave, app);

    int quality = 90;
    output_group->add_option("-q,--quality", quality, "JPEG quality (1-100)");
    CLI_GUI::gui_widget(output_group->get_option("--quality"),
                        CLI_GUI::WidgetType::SliderInt, app);
    CLI_GUI::gui_min(output_group->get_option("--quality"), 1, app);
    CLI_GUI::gui_max(output_group->get_option("--quality"), 100, app);

    std::string format = "png";
    output_group->add_option("-f,--format", format, "Output format");
    CLI_GUI::gui_values(output_group->get_option("--format"),
                        {"png", "jpg", "webp", "bmp", "tiff"}, app);

    // ── Group: Filters (ungrouped) ──
    bool grayscale = false;
    app.add_flag("-g,--grayscale", grayscale, "Convert to grayscale");

    bool sharpen = false;
    app.add_flag("--sharpen", sharpen, "Apply sharpen filter");

    app.set_main([&]() {
        std::cout << "Processing: " << source << std::endl;
        std::cout << "  Target size: " << width << "x" << height << std::endl;
        std::cout << "  Output: " << dest << " (" << format
                  << ", quality=" << quality << ")" << std::endl;
        if (grayscale) std::cout << "  Filter: grayscale" << std::endl;
        if (sharpen)   std::cout << "  Filter: sharpen" << std::endl;
    });

    CLI_GUI_PARSE(app, argc, argv);
    return 0;
}
