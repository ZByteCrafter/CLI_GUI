#include <CLI_GUI/CLI_GUI.hpp>

#ifdef CLI_GUI_HAS_GUI

#include <CLI_GUI/detail/BackendGLFW.hpp>
#include <CLI_GUI/detail/LayoutEngine.hpp>
#include <CLI_GUI/detail/ConsoleCapture.hpp>
#include <thread>
#include <sstream>
#include <iostream>

namespace CLI_GUI {

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
                    // No callback -- GUI will just close
                    console.quit_requested = true;
                }
            }

            // Handle window close (X button)
            if (backend.should_close()) {
                console.quit_requested = true;
            }
        }

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
