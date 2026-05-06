# AGENTS.md — CLI11_GUI

## Build commands

```powershell
# Header-only tests (no GUI deps, fast)
cmake -B build -DCLI_GUI_BUILD_TESTS=ON -DCLI_GUI_ENABLE_GUI=OFF
cmake --build build
build/tests/Debug/cli_gui_tests.exe

# GUI build (compiles ImGui+GLFW from vendor/)
cmake -B build_gui -DCLI_GUI_ENABLE_GUI=ON
cmake --build build_gui

# Examples (implies GUI ON)
cmake -B build_examples -DCLI_GUI_BUILD_EXAMPLES=ON
cmake --build build_examples

# Run one test via ctest
ctest --test-dir build -R "widget_type"
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
- 33 tests as of current.

## GUI code gating

All GUI implementation is behind `#ifdef CLI_GUI_HAS_GUI`:

```
include/CLI_GUI/GuiLauncher.hpp        ← launch_gui() declaration
include/CLI_GUI/detail/BackendGLFW.hpp ← ImGui+GLFW lifecycle + CJK font loading
include/CLI_GUI/detail/LayoutEngine.hpp← widget/form rendering + console panel
include/CLI_GUI/detail/ConsoleCapture.hpp← cout redirect (thread-safe streambuf)
include/CLI_GUI/detail/FileDialog.hpp  ← native Win32 file/folder dialogs
include/CLI_GUI/detail/Win32SuppressConsole.hpp ← Windows console suppression
src/CLI_GUI.cpp                        ← launch_gui() + flush_gui_to_cli + fallback
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

## Hard-earned gotchas

### CLI11 `get_subcommands()` — ALWAYS use filter overload

`app.get_subcommands()` returns `parsed_subcommands_` (only populated after `parse()` — empty in GUI mode). Use the filter overload which iterates registered subcommands:

```cpp
// WRONG — empty in GUI mode (returns parsed_subcommands_)
auto subs = app.get_subcommands();

// RIGHT — returns all registered subcommands
auto subs = app.get_subcommands([](CLI::App*) { return true; });
```

This applies everywhere: guards, loops, flush_gui_to_cli.

### `render_option` — `initialized` must be set unconditionally

```cpp
// WRONG — initialized stays false when opt->results() is empty (GUI mode)
if (!mut_meta.initialized && !opt->results().empty()) {
    // ... copy from results ...
    mut_meta.initialized = true;  // never reached in GUI mode!
}

// RIGHT — initialized set regardless of results
if (!mut_meta.initialized) {
    if (!opt->results().empty()) { /* copy from results */ }
    mut_meta.initialized = true;  // always set on first render
}
```

If `initialized` stays false, `flush_gui_to_cli` skips the option entirely.

### CLI11 `App` constructor — single arg is description, not name

```cpp
App{"Hello GUI"}  // description="Hello GUI", name=""  → gui_title() returns ""
App{"Desc", "Hello GUI"}  // description="Desc", name="Hello GUI"  → gui_title() returns "Hello GUI"
```

`gui_title()` fallback chain: custom title → name → description.

### ConsoleState uses deque with snapshot, not queue with pop

`ConsoleState::buffer` is `std::deque` (max 1000). `snapshot()` returns a copy under `mutable std::mutex`. `render_console()` iterates the snapshot without consuming lines. Never pop/discard messages.

### `flush_gui_to_cli` — active subcommand only

When Run is clicked, `flush_gui_to_cli(app, active_subcommand)` collects root options + only the currently active subcommand's options (inserting the subcommand name as a positional arg before its options). Other subcommand options are skipped.

### ImGui ID collision — use PushID/PopID for repeated widgets

Multiple FileOpen/FileSave/DirPicker widgets render the same `InputText("##file_input", ...)`. In ImGui, identical labels mean identical IDs → clicking one button may trigger another. Always wrap with `PushID(opt)` / `PopID()`:

```cpp
ImGui::PushID(opt);
ImGui::InputText("##file_input", buf, size);
ImGui::PopID();
```

### Input field + button layout — use SetNextItemWidth(-X)

The correct pattern for a text field with a `[Browse]` button on the same line:

```cpp
float btn_w = 80;
ImGui::SetNextItemWidth(-(btn_w + ImGui::GetStyle().ItemSpacing.x));
ImGui::InputText("##input", buf, size);  // negative = offset from right edge
ImGui::SameLine();
ImGui::Button("Browse", ImVec2(btn_w, 0));
```

Do NOT use `PushItemWidth(GetContentRegionAvail().x - btn_w)` — the label consumes unknown space.

### File dialogs use NULL owner on Windows

`GetActiveWindow()` returns the wrong HWND for GLFW windows. Pass `nullptr` as `hwndOwner` in `OPENFILENAMEA`/`BROWSEINFOA`. The dialog appears as a standalone top-level window.

### `set_callback` now runs in CLI mode

Both branches of `CLI_GUI_PARSE` check `gui_callback()` before `gui_main()`. Priority: callback > main, CLI/GUI consistent. Callback in CLI mode runs synchronously without progress bar.

## vendor/ is the sole dependency source

All three dependencies live in `vendor/` and are built via `add_subdirectory`:

- `vendor/CLI11/` — single-header `CLI/CLI.hpp` + minimal `CMakeLists.txt` for `CLI11::CLI11` target
- `vendor/imgui/` — Dear ImGui `.cpp` files → `imgui` STATIC target
- `vendor/glfw/` — GLFW `.c` files with platform branches (WIN32/APPLE/else) → `glfw` STATIC target

ImGui GLFW+OpenGL3 backend `.cpp` files are NOT their own target — they are compiled directly as sources of `CLI_GUI_Gui` in the root `CMakeLists.txt`.

## Platform quirks

- **Shell is PowerShell 5.1**: `&&` does not work. Run commands separately.
- **Build output paths**: `build/<subdir>/Debug/<name>.exe` (MSVC Debug). Adjust for other generators.
- **Windows console suppression**: examples use `/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup`. `cli_gui_init_console()` must be called as the first line of `main()` to attach to parent console when launched from terminal.
- **Icon**: GLFW windows need `glfwSetWindowIcon()` explicitly — the `.exe` embedded icon is NOT auto-applied to the window. BackendGLFW constructor sets a default 32x32 RGBA icon via embedded pixel data.

## Quick smoke tests

```powershell
# Tests (33)
build/tests/Debug/cli_gui_tests.exe

# CLI mode
build_examples/examples/Debug/01_basic.exe -n "CLI" -c 3 -u
# Expected: prints HELLO, CLI! three times

# GUI mode (opens a window — needs display)
build_examples/examples/Debug/01_basic.exe
```
