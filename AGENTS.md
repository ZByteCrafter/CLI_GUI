# AGENTS.md — CLI11_GUI

## Build commands

```powershell
# Header-only tests (no GUI deps, fast)
cmake -B build_tests -DCLI_GUI_BUILD_TESTS=ON -DCLI_GUI_ENABLE_GUI=OFF
cmake --build build_tests
build_tests/tests/Debug/cli_gui_tests.exe

# GUI build (compiles ImGui+GLFW from vendor/)
cmake -B build_gui -DCLI_GUI_ENABLE_GUI=ON
cmake --build build_gui

# Examples (implies GUI ON)
cmake -B build_ex -DCLI_GUI_BUILD_EXAMPLES=ON
cmake --build build_ex

# Run one test via ctest
ctest --test-dir build_tests -R "widget_type"
```

## Two build modes

| Mode | CMake flag | Library shape | Dependencies |
|------|-----------|---------------|-------------|
| Header-only | `CLI_GUI_ENABLE_GUI=OFF` | `CLI_GUI` INTERFACE | CLI11 only (vendor/) |
| With GUI | `CLI_GUI_ENABLE_GUI=ON` (default) | `CLI_GUI` INTERFACE + `CLI_GUI_Gui` STATIC | CLI11 + ImGui + GLFW (all vendor/) |

Users always link `CLI_GUI::CLI_GUI`. When GUI is on, `CLI_GUI_Gui` is injected into the `CLI_GUI` target automatically.

## Test framework

`tests/catch2.hpp` is a **custom minimal test runner**, NOT real Catch2. It provides `TEST_CASE(name, tag)` and `REQUIRE(expr)` macros compatible with Catch2 syntax.

- Only `test_app.cpp` defines `#define CATCH_CONFIG_MAIN` before including `catch2.hpp`.
- New test files must NOT define `CATCH_CONFIG_MAIN` — all `.cpp` files compile into one executable `cli_gui_tests`.
- The runner catches `std::exception` (not Catch2-specific types).

## GUI code gating

All GUI implementation is behind `#ifdef CLI_GUI_HAS_GUI`:

```
include/CLI_GUI/GuiLauncher.hpp        ← launch_gui() declaration
include/CLI_GUI/detail/BackendGLFW.hpp ← ImGui+GLFW lifecycle
include/CLI_GUI/detail/LayoutEngine.hpp← widget/form rendering
include/CLI_GUI/detail/ConsoleCapture.hpp← cout redirect
src/CLI_GUI.cpp                        ← launch_gui() implementation + fallback
```

Without `CLI_GUI_HAS_GUI`, `CLI_GUI.hpp` does NOT include `GuiLauncher.hpp`, and `CLI_GUI_PARSE` is a plain `CLI::App::parse` call.

## WidgetType — watch for circular deps

`WidgetType` was extracted to `include/CLI_GUI/WidgetType.hpp` to break the App↔WidgetMapper cycle:

```
WidgetType.hpp  ← enum definition (no deps)
    ↑
WidgetMapper.hpp ← #include <CLI_GUI/WidgetType.hpp>  (to_string, type traits)
    ↑
App.hpp         ← #include <CLI_GUI/WidgetMapper.hpp>  (needs WidgetType + free functions)
    ↑
CLI_GUI.hpp     ← #include <CLI_GUI/App.hpp>
```

Never put `#include <CLI_GUI/App.hpp>` in `WidgetType.hpp` or `WidgetMapper.hpp`.

## vendor/ is the sole dependency source

All three dependencies live in `vendor/` and are built via `add_subdirectory`:

- `vendor/CLI11/` — single-header `CLI/CLI.hpp` + minimal `CMakeLists.txt` for `CLI11::CLI11` target
- `vendor/imgui/` — Dear ImGui `.cpp` files → `imgui` STATIC target
- `vendor/glfw/` — GLFW `.c` files with platform branches (WIN32/APPLE/else) → `glfw` STATIC target

ImGui GLFW+OpenGL3 backend `.cpp` files are NOT their own target — they are compiled directly as sources of `CLI_GUI_Gui` in the root `CMakeLists.txt:43-45`.

## Platform quirks

- **Shell is PowerShell 5.1**: `&&` does not work. Chain commands with `;` or run separately.
- **Build output paths**: `build/<subdir>/Debug/<name>.exe` (MSVC Debug). Adjust for other generators.

## Quick smoke tests

```powershell
# CLI mode
build_ex/examples/Debug/01_basic.exe -n "CLI" -c 3 -u
# Expected: prints HELLO, CLI! three times

# GUI mode (opens a window — needs display)
build_ex/examples/Debug/01_basic.exe
```
