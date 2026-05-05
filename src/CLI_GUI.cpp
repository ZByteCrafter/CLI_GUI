#include <CLI_GUI/CLI_GUI.hpp>

#ifdef CLI_GUI_HAS_GUI

#include <CLI_GUI/detail/BackendGLFW.hpp>
#include <CLI_GUI/detail/LayoutEngine.hpp>
#include <CLI_GUI/detail/ConsoleCapture.hpp>
#include <thread>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>

namespace CLI_GUI {

/// Collect values from per-option GUI state and write them back to CLI11.
/// Builds a fake argv and calls app.parse() to trigger validation and callbacks.
static void flush_gui_to_cli(App& app) {
    std::vector<std::string> args;
    args.push_back("gui"); // argv[0] placeholder

    // Helper: collect options from an App (including subcommands)
    auto collect = [&args](App& a) {
        for (auto* opt : a.get_options()) {
            const auto& meta = a.gui_meta(opt);
            // Extract the first name (e.g. "--count" from "--count,-c")
            std::string raw_name = opt->get_name();
            if (raw_name.empty()) continue;
            auto comma = raw_name.find(',');
            std::string name = (comma != std::string::npos)
                               ? raw_name.substr(0, comma) : raw_name;

            WidgetType wt = meta.widget_type;
            if (wt == WidgetType::Auto) {
                wt = (opt->get_expected_min() == 0)
                     ? WidgetType::Checkbox : WidgetType::InputText;
            }

            switch (wt) {
            case WidgetType::Checkbox:
                if (meta.bool_state) {
                    args.push_back(name);
                }
                break;
            case WidgetType::InputText:
            case WidgetType::Password:
            case WidgetType::FileOpen:
            case WidgetType::FileSave:
            case WidgetType::DirPicker:
            case WidgetType::CodeEditor:
            case WidgetType::IpAddress:
                if (meta.text_buf[0] != 0) {
                    args.push_back(name);
                    args.push_back(meta.text_buf);
                }
                break;
            case WidgetType::InputInt:
            case WidgetType::SpinInt:
            case WidgetType::SliderInt:
                args.push_back(name);
                args.push_back(std::to_string(meta.int_state));
                break;
            case WidgetType::InputFloat:
            case WidgetType::SpinFloat:
            case WidgetType::SliderFloat:
                args.push_back(name);
                args.push_back(std::to_string(meta.float_state));
                break;
            case WidgetType::Combo:
            case WidgetType::Radio:
                if (!meta.values.empty() && meta.combo_current >= 0 &&
                    static_cast<size_t>(meta.combo_current) < meta.values.size()) {
                    args.push_back(name);
                    args.push_back(meta.values[meta.combo_current]);
                }
                break;
            default:
                break;
            }
        }
    };

    collect(app);
    for (auto* sub : app.get_subcommands()) {
        // subcommands are CLI::App*, need to cast or handle separately.
        // For now, subcommand options' metadata is on root app,
        // so the collect() above already covers them.
    }

    // Build char* array and parse
    std::vector<const char*> argv;
    for (auto& a : args) argv.push_back(a.c_str());
    if (argv.size() > 1) {
        try {
            app.parse(static_cast<int>(argv.size()), const_cast<char**>(argv.data()));
        } catch (const CLI::ParseError& e) {
            // GUI already validated; parse errors are unexpected but non-fatal
            std::cerr << "CLI_GUI: parse error after GUI input: " << e.what() << std::endl;
        }
    }
}

void launch_gui(App& app, int /*argc*/, char** /*argv*/) {
    // Try to create GUI window. If it fails (no display), fall back to CLI.
    try {
        detail::BackendGLFW backend(app.gui_title(), app.gui_width(), app.gui_height());
        detail::ConsoleState console;
        console.show_console = app.gui_show_console();
        console.console_height = app.gui_console_height();

        // Redirect cout to console panel
        detail::CoutRedirect cout_redirect(
            [&console](const std::string& line) {
                console.push_line(line);
            }
        );

        std::thread worker;

        while (!backend.should_close() && !console.quit_requested) {
            backend.new_frame();

            // Main window fills entire client area
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
            ImGui::Begin("##MainWindow", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

            // Title
            ImGui::Text("%s", app.gui_title().c_str());
            ImGui::Separator();

            // Render subcommands as tabs, or plain options if no subcommands
            if (!app.get_subcommands().empty()) {
                detail::render_subcommands(app, console);
            } else {
                detail::render_options(app);
            }

            // Console panel
            detail::render_console(console);

            // Bottom bar (progress + Run/Cancel/Quit)
            detail::render_bottom_bar(app, console);

            ImGui::End();
            backend.render();

            // Handle Run button
            if (console.run_requested && !console.running) {
                console.run_requested = false;

                // Flush GUI values back to CLI11 before executing
                flush_gui_to_cli(app);

                auto cb = app.gui_callback();
                if (cb) {
                    console.running = true;
                    app.reset_cancel();
                    app.update_progress(0.0f);
                    console.push_line("[INFO] Running...");
                    if (worker.joinable()) worker.join();
                    worker = std::thread([cb, &app, &console]() {
                        cb();
                        console.running = false;
                        if (!app.is_cancelled())
                            console.push_line("[DONE] Complete.");
                        else
                            console.push_line("[CANCEL] Cancelled by user.");
                    });
                } else {
                    // No callback -- flush was done, GUI closes
                    console.quit_requested = true;
                }
            }

            // Handle window close (X button)
            if (backend.should_close()) {
                console.quit_requested = true;
            }
        }

        // On exit, flush values one more time (in case user quit without Run)
        flush_gui_to_cli(app);

        // Wait for worker thread
        app.request_cancel();
        if (worker.joinable()) {
            worker.join();
        }
    } catch (const std::exception& e) {
        // Fallback: if GUI fails (no display, no GPU), run in CLI mode
        std::cerr << "CLI_GUI: GUI unavailable (" << e.what() << "). Falling back to CLI." << std::endl;
    }
}

} // namespace CLI_GUI

#endif // CLI_GUI_HAS_GUI
