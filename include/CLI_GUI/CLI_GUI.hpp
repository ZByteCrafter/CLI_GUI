#pragma once

// CLI11_GUI -- Single public include header.
// When CLI_GUI_HAS_GUI is defined, GuiLauncher is included.

#include <CLI/CLI.hpp>
#include <CLI_GUI/App.hpp>
#include <CLI_GUI/WidgetMapper.hpp>

#ifdef CLI_GUI_HAS_GUI
#include <CLI_GUI/GuiLauncher.hpp>
#endif
