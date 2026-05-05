#pragma once

#ifdef CLI_GUI_HAS_GUI

namespace CLI_GUI {

class App;

/// Launch the GUI window. Defined in src/CLI_GUI.cpp.
void launch_gui(App& app, int argc, char** argv);

} // namespace CLI_GUI

#endif // CLI_GUI_HAS_GUI
