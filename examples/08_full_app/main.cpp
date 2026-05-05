// Example 08: Full-featured application
// Combines: subcommands, option groups, color pickers, file dialogs, validators,
//           progress bar, console output, and cancel support.
#include <CLI_GUI/CLI_GUI.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>

int main(int argc, char** argv) {
    CLI_GUI::App app{"Media Converter Suite"};
    app.gui_size(960, 720);

    // ── Subcommand: encode ──
    auto* encode = app.add_subcommand("encode", "Encode a media file");

    auto* enc_input_grp = encode->add_option_group("Input");
    std::string input_file;
    enc_input_grp->add_option("-i,--input", input_file, "Input file")->required();
    CLI_GUI::gui_label(enc_input_grp->get_option("--input"), "Input File", app);
    CLI_GUI::gui_widget(enc_input_grp->get_option("--input"),
                        CLI_GUI::WidgetType::FileOpen, app);

    auto* enc_output_grp = encode->add_option_group("Output");
    std::string output_file = "output.mp4";
    enc_output_grp->add_option("-o,--output", output_file, "Output file");
    CLI_GUI::gui_label(enc_output_grp->get_option("--output"), "Output File", app);
    CLI_GUI::gui_widget(enc_output_grp->get_option("--output"),
                        CLI_GUI::WidgetType::FileSave, app);

    std::string codec = "h264";
    enc_output_grp->add_option("--codec", codec, "Video codec");
    CLI_GUI::gui_values(enc_output_grp->get_option("--codec"),
                        {"h264", "h265", "av1", "vp9", "prores"}, app);

    int bitrate = 5000;
    enc_output_grp->add_option("-b,--bitrate", bitrate, "Bitrate (kbps)");
    CLI_GUI::gui_widget(enc_output_grp->get_option("--bitrate"),
                        CLI_GUI::WidgetType::SliderInt, app);
    CLI_GUI::gui_min(enc_output_grp->get_option("--bitrate"), 100, app);
    CLI_GUI::gui_max(enc_output_grp->get_option("--bitrate"), 50000, app);

    auto* enc_quality_grp = encode->add_option_group("Quality");
    int crf = 23;
    enc_quality_grp->add_option("--crf", crf, "Constant Rate Factor (0-51)");
    CLI_GUI::gui_widget(enc_quality_grp->get_option("--crf"),
                        CLI_GUI::WidgetType::SliderInt, app);
    CLI_GUI::gui_min(enc_quality_grp->get_option("--crf"), 0, app);
    CLI_GUI::gui_max(enc_quality_grp->get_option("--crf"), 51, app);

    bool two_pass = false;
    enc_quality_grp->add_flag("--two-pass", two_pass, "Use two-pass encoding");

    // ── Subcommand: resize ──
    auto* resize = app.add_subcommand("resize", "Resize media files");

    auto* rsz_group = resize->add_option_group("Dimensions");
    int new_width = 1920;
    rsz_group->add_option("-W,--width", new_width, "Target width");
    CLI_GUI::gui_min(rsz_group->get_option("--width"), 1, app);
    CLI_GUI::gui_max(rsz_group->get_option("--width"), 7680, app);

    int new_height = 1080;
    rsz_group->add_option("-H,--height", new_height, "Target height");
    CLI_GUI::gui_min(rsz_group->get_option("--height"), 1, app);
    CLI_GUI::gui_max(rsz_group->get_option("--height"), 4320, app);

    std::string resize_input;
    resize->add_option("-i,--input", resize_input, "Input file")->required();
    CLI_GUI::gui_widget(resize->get_option("--input"),
                        CLI_GUI::WidgetType::FileOpen, app);

    std::string resize_output = "resized.mp4";
    resize->add_option("-o,--output", resize_output, "Output file");
    CLI_GUI::gui_widget(resize->get_option("--output"),
                        CLI_GUI::WidgetType::FileSave, app);

    // ── Shared options ──
    int threads = 4;
    app.add_option("-j,--jobs", threads, "Parallel threads");
    CLI_GUI::gui_widget(app.get_option("--jobs"), CLI_GUI::WidgetType::SliderInt, app);
    CLI_GUI::gui_min(app.get_option("--jobs"), 1, app);
    CLI_GUI::gui_max(app.get_option("--jobs"), 16, app);

    bool verbose = false;
    app.add_flag("-v,--verbose", verbose, "Verbose output");

    // ── Callback for encoding (simulated long task) ──
    app.set_callback([&]() {
        int total = 20;
        for (int i = 1; i <= total; ++i) {
            if (app.is_cancelled()) {
                std::cout << "[CANCEL] Aborted at " << i << "/" << total << std::endl;
                return;
            }
            std::cout << "[" << i << "/" << total << "] Processing frame..."
                      << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            app.update_progress(static_cast<float>(i) / total);
        }
        std::cout << "[OK] Encoding complete." << std::endl;
    });

    // ── Main logic (CLI mode, or post-callback) ──
    app.set_main([&]() {
        if (encode->parsed()) {
            std::cout << "Encode mode selected" << std::endl;
            std::cout << "  Input: " << input_file << std::endl;
            std::cout << "  Output: " << output_file << std::endl;
            std::cout << "  Codec: " << codec << std::endl;
            std::cout << "  Bitrate: " << bitrate << " kbps" << std::endl;
            std::cout << "  CRF: " << crf << std::endl;
            std::cout << "  Two-pass: " << (two_pass ? "yes" : "no") << std::endl;
        }
        if (resize->parsed()) {
            std::cout << "Resize mode selected" << std::endl;
            std::cout << "  Input: " << resize_input << std::endl;
            std::cout << "  Output: " << resize_output << std::endl;
            std::cout << "  Size: " << new_width << "x" << new_height << std::endl;
        }
        if (threads > 1) std::cout << "Using " << threads << " threads" << std::endl;
    });

    CLI_GUI_PARSE(app, argc, argv);
    return 0;
}
