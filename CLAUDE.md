# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Header-only tests (no GUI deps, fast — primary dev loop)
cmake -B build -DCLI_GUI_BUILD_TESTS=ON -DCLI_GUI_ENABLE_GUI=OFF
cmake --build build
build/tests/Debug/cli_gui_tests.exe

# GUI build (compiles ImGui+GLFW from vendor/)
cmake -B build_gui -DCLI_GUI_BUILD_TESTS=ON -DCLI_GUI_ENABLE_GUI=ON
cmake --build build_gui
build_gui/tests/Debug/cli_gui_tests.exe

# Examples (implies GUI ON)
cmake -B build_examples -DCLI_GUI_BUILD_EXAMPLES=ON
cmake --build build_examples

# Run single test by ctest name pattern
ctest --test-dir build -R "widget_type"

# CLI smoke test
build/examples/Debug/01_basic.exe -n "CLI" -c 3 -u
# Expected: prints HELLO, CLI! three times
```

## Architecture

**CLI11_GUI** extends CLI11 so the same code supports CLI and GUI modes. When launched with args, it behaves like native CLI11. When launched without args (double-click), it opens a Dear ImGui window.

Three-layer design:

1. **Header-only `CLI_GUI::App`** (`include/CLI_GUI/App.hpp`) — inherits `CLI::App`, stores per-option GUI metadata (`OptionGuiMeta`) in a side map keyed by `CLI::Option*`. Zero GUI dependency. The `CLI_GUI_PARSE` macro dispatches to GUI or CLI path.

2. **GUI implementation** (behind `#ifdef CLI_GUI_HAS_GUI`, compiled into `CLI_GUI_Gui` static lib):
   - `detail/BackendGLFW.hpp` — ImGui+GLFW+OpenGL3 lifecycle, CJK font auto-detection, drag-drop via `glfwSetDropCallback`
   - `detail/LayoutEngine.hpp` — widget rendering (`render_option`), subcommand tabs, option group collapsible panels, console panel, required-field validation, bottom bar
   - `detail/ConsoleCapture.hpp` — thread-safe `std::cout` redirect to GUI console
   - `detail/FileDialog.hpp` — native Win32 file/folder dialogs
   - `src/CLI_GUI.cpp` — `launch_gui()` event loop, `flush_gui_to_cli()` (builds synthetic argv from widget state), worker thread management

3. **`flush_gui_to_cli`** — when "Run" is clicked, collects widget state from `OptionGuiMeta`, builds fake argv, calls `app.parse()` to trigger CLI11 validation. Only flushes the active subcommand's options.

Key flow: `CLI_GUI_PARSE(app, argc, argv)` → argc<=1: `launch_gui()` → ImGui window → user clicks Run → `flush_gui_to_cli()` → `app.parse(synthetic argv)` → execute callback/main in worker thread.

## Two Build Modes

| Mode | CMake flag | Library shape | Dependencies |
|------|-----------|---------------|-------------|
| Header-only | `CLI_GUI_ENABLE_GUI=OFF` | `CLI_GUI` INTERFACE | CLI11 only |
| With GUI | `CLI_GUI_ENABLE_GUI=ON` (default) | `CLI_GUI` INTERFACE + `CLI_GUI_Gui` STATIC | CLI11 + ImGui + GLFW |

Users always link `CLI_GUI::CLI_GUI`. When GUI is on, `CLI_GUI_Gui` is injected automatically.

## Test Framework

`tests/catch2.hpp` is a **custom minimal test runner**, NOT real Catch2. Provides `TEST_CASE(name, tag)` and `REQUIRE(expr)`.

- Only `test_app.cpp` defines `#define CATCH_CONFIG_MAIN`. New test files must NOT define it.
- All `.cpp` files compile into one executable `cli_gui_tests`.
- `test_flush_real.cpp` tests the real `flush_gui_to_cli()` function — it's wrapped in `#ifdef CLI_GUI_HAS_GUI` so it only runs in GUI builds.
- Run single test by ctest pattern: `ctest --test-dir build -R "widget_type"`

## Include Dependency Chain (no cycles)

```
WidgetType.hpp       ← enum only, no deps
    ↑
WidgetMapper.hpp     ← to_string, compile-time type→widget traits
    ↑
App.hpp              ← CLI_GUI::App, OptionGuiMeta, free functions, CLI_GUI_PARSE macro
    ↑
CLI_GUI.hpp          ← single public entry point; conditionally includes GuiLauncher.hpp
```

Never put `#include <CLI_GUI/App.hpp>` in `WidgetType.hpp` or `WidgetMapper.hpp`.

## Critical Gotchas

### CLI11 `get_subcommands()` — ALWAYS use filter overload

`app.get_subcommands()` returns parsed subcommands (empty in GUI mode). Use the filter overload:
```cpp
auto subs = app.get_subcommands([](CLI::App*) { return true; });
```

### `initialized` must be set unconditionally in render_option

```cpp
// WRONG — initialized stays false in GUI mode (no CLI results)
if (!meta.initialized && !opt->results().empty()) {
    // ...
    meta.initialized = true;
}

// RIGHT
if (!meta.initialized) {
    if (!opt->results().empty()) { /* copy from results */ }
    meta.initialized = true;
}
```

### Positional args in flush_gui_to_cli

Options without `-` prefix are positional. Don't push the name to argv, just push the value. For multi-value positional args (`expected_max > 1`), `push_value` splits text by spaces. Auto-inferred widget type for positional args must remain `InputText` (not `List`) so the space-splitting logic works.

### `flush_gui_to_cli` Auto inference

```cpp
if (wt == WidgetType::Auto) {
    if (opt->get_expected_min() == 0) wt = Checkbox;
    else if (!meta.values.empty())    wt = Combo;
    else                              wt = InputText;
}
```
Do NOT add `get_expected_max() > 1 → List` — it breaks positional multi-value args. Vector options must use `gui_widget(opt, WidgetType::List, app)` explicitly.

### `gui_meta` field sync before flush

For `Auto`-inferred `InputText` options that are actually numeric, `flush_gui_to_cli` syncs `int_state`/`float_state` into `text_buf` before flushing. This handles sliders/spinners whose values live in `int_state`/`float_state` rather than `text_buf`.

### CLI11 `add_option_group()` creates hidden subcommands

Filter option groups from real subcommands by `sub->get_name().empty()`, not by `sub->get_group()`.

### Option group options are flushed correctly

`flush_gui_to_cli` iterates both direct options and option groups (empty-name subcommands) via `flush_option()` helper. Options registered via `add_option_group()` are included in the synthetic argv.

### ImGui ID collisions

Multiple widgets with the same label create ID collisions. Always wrap with `PushID(opt)` / `PopID()`.

### UTF-8 safe truncation

Use `detail::utf8_strncpy(dst, src, dst_size)` instead of `strncpy` when copying UTF-8 strings into fixed-size buffers. It truncates at valid UTF-8 boundaries to avoid splitting multi-byte characters.

### `--help` and parse errors skip callback/main

After `app.parse()`, check `get_help_ptr()->count()`. If help was requested, don't execute callbacks. The `CLI_GUI_PARSE` macro also tracks parse success — if `parse()` throws (e.g. `RequiredError`, `ArgumentMismatch`), the callback is skipped entirely to avoid running with undefined state.

### `set_callback` priority

`callback > main` in both CLI and GUI modes. Both branches of `CLI_GUI_PARSE` check `gui_callback()` first. Callback in CLI mode runs synchronously without progress bar.

### CLI11 `App` constructor — single arg is description, not name

```cpp
App{"Hello GUI"}           // description="Hello GUI", name=""  → gui_title() returns ""
App{"Desc", "Hello GUI"}   // description="Desc", name="Hello GUI"  → gui_title() returns "Hello GUI"
```

`gui_title()` fallback chain: custom title → name → description.

### GUI code gating

All GUI implementation is behind `#ifdef CLI_GUI_HAS_GUI`. Without it, `CLI_GUI.hpp` does NOT include `GuiLauncher.hpp`, and `CLI_GUI_PARSE` is a plain `CLI::App::parse` call. The `CLI_GUI_Gui` static lib (and ImGui/GLFW) only compiles when GUI is enabled.

## Platform Notes

- Shell is bash on this machine (Git Bash on Windows). Build output: `build/<subdir>/Debug/<name>.exe` (MSVC).
- Windows examples use `/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup`. Call `cli_gui_init_console()` as first line of `main()`.
- File dialogs (`FileDialog.hpp`) are Win32-only. On other platforms, `Browse` buttons print a stderr warning.
- `imgui.ini` is auto-generated and in `.gitignore`.
- GLFW windows need `glfwSetWindowIcon()` explicitly — the `.exe` icon is not auto-applied.
- `vendor/` is the sole dependency source (CLI11, ImGui, GLFW) — all built via `add_subdirectory`.

## Character Encoding (Windows)

On Chinese Windows, CLI `argv` is GBK, but ImGui uses UTF-8. Two encoding modes:

- **GBK (default)**: Callbacks receive GBK. Library auto-converts internally (FileDialog UTF-8→GBK, ConsoleCapture GBK→UTF-8). User code needs zero conversion.
- **UTF-8**: Callbacks receive UTF-8. User must call `CLI_GUI::utf8_to_ansi()` before passing paths to `std::filesystem` / Win32 APIs.

Set via `app.gui_encoding(CLI_GUI::Encoding::Utf8)` in `App.hpp`. Utility functions in `Encoding.hpp` (`utf8_to_ansi`, `ansi_to_utf8`).

Key files:
- `include/CLI_GUI/Encoding.hpp` — conversion utilities
- `include/CLI_GUI/detail/FileDialog.hpp` — uses Win32 W APIs, returns UTF-8
- `src/CLI_GUI.cpp` — `flush_gui_to_cli()` converts UTF-8→GBK in GBK mode; `launch_gui()` console callback converts GBK→UTF-8 in GBK mode
- `include/CLI_GUI/App.hpp` — `CLI_GUI_PARSE` macro converts argv GBK→UTF-8 in UTF-8 mode

## Quick Smoke Tests

```bash
# All 49 tests
build/tests/Debug/cli_gui_tests.exe

# CLI mode
build/examples/Debug/01_basic.exe -n "CLI" -c 3 -u
# Expected: prints HELLO, CLI! three times

# Positional args
build/examples/Debug/09_positional.exe Alice -g Hi -c 2
# Expected: Hi, Alice! twice

# GUI mode (opens a window — needs display)
build/examples/Debug/01_basic.exe
```
