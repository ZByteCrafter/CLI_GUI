#include <CLI_GUI/CLI_GUI.hpp>
#include <CLI_GUI/detail/Win32SuppressConsole.hpp>
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char** argv) {
    cli_gui_init_console();
    CLI_GUI::App app{"Long Task Demo"};

    int steps = 10;
    app.add_option("-s,--steps", steps, "Number of steps");
    CLI_GUI::gui_widget(app.get_option("--steps"), CLI_GUI::WidgetType::SliderInt, app);
    CLI_GUI::gui_min(app.get_option("--steps"), 1, app);
    CLI_GUI::gui_max(app.get_option("--steps"), 50, app);

    app.set_callback([&]() {
        for (int i = 1; i <= steps; ++i) {
            std::cout << "Step " << i << "/" << steps << " processing..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            app.update_progress(static_cast<float>(i) / static_cast<float>(steps));
        }
        std::cout << "All done!" << std::endl;
    });

    CLI_GUI_PARSE(app, argc, argv);

    return 0;
}
