#pragma once

#ifdef CLI_GUI_HAS_GUI

#include <string>

namespace CLI_GUI {

class App;

/// Launch the GUI window. Defined in src/CLI_GUI.cpp.
void launch_gui(App& app, int argc, char** argv);

/// Flush GUI widget state back to CLI11 via synthetic argv. Defined in src/CLI_GUI.cpp.
void flush_gui_to_cli(App& app, const std::string& active_subcommand = "");

} // namespace CLI_GUI

#endif // CLI_GUI_HAS_GUI
