# Required Options Handling + Drag-drop Demo — Implementation Plan

> **Date**: 2026-05-06
> **Scope**: Visual feedback for required options in GUI; drag-drop file demo in examples

## 1. Required Options GUI Feedback

### 1a. Red asterisk on labels

**File**: `include/CLI_GUI/detail/LayoutEngine.hpp` — `render_option()`

At the top of `render_option`, after label construction, append `*` if required:

```cpp
std::string label = meta.label.empty() ? raw_name : meta.label;
if (opt->get_required()) {
    label += " *";  // red star indicator
}
```

Then render the `*` in red. For the label rendering that uses `ImGui::TextUnformatted`, push a red color for just the `*`. Or simpler: use `ImGui::PushStyleColor` before the label.

Alternative: use a separate `ImGui::SameLine(); ImGui::TextColored(red, "*");` after the label text.

### 1b. Gray out Run button if required options are unfilled

**File**: `include/CLI_GUI/detail/LayoutEngine.hpp` — `render_bottom_bar()`

Add a helper that checks if all required options across root + active subcommand are filled:

```cpp
static bool all_required_filled(App& app, const std::string& active_subcommand) {
    auto check = [](App& a, CLI::App* cli_app) -> bool {
        for (auto* opt : cli_app->get_options()) {
            if (!opt->get_required()) continue;
            const auto& meta = a.gui_meta(opt);
            // Check if the option has a value
            if (!meta.initialized) return false;
            // For text fields, check if empty
            WidgetType wt = meta.widget_type;
            if (wt == WidgetType::Auto)
                wt = (opt->get_expected_min() == 0) ? WidgetType::Checkbox : WidgetType::InputText;
            if (wt == WidgetType::InputText) {
                if (meta.text_buf[0] == 0) return false;
            }
        }
        return true;
    };
    if (!check(app, &app)) return false;
    if (!active_subcommand.empty()) {
        for (auto* sub : app.get_subcommands([](CLI::App*){ return true; })) {
            if (sub->get_name() == active_subcommand) {
                return check(app, sub);
            }
        }
    }
    return true;
}
```

In `render_bottom_bar`, before the Run button:

```cpp
bool can_run = all_required_filled(app, console.active_subcommand);
if (!can_run) {
    ImGui::BeginDisabled();
}
if (ImGui::Button("Run", ImVec2(120, 0))) { ... }
if (!can_run) {
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1,0.2f,0.2f,1), "Required fields empty");
}
```

### 1c. Better error in flush_gui_to_cli

**File**: `src/CLI_GUI.cpp` — `flush_gui_to_cli()`

Currently catch block just prints to cerr. Add a push_line to the console (but we don't have console access). Simpler: don't swallow the exception; let it propagate and let the caller handle it.

Actually, the simplest fix: in the catch block, instead of swallowing, push a message to args that will force a re-parse with what we have. Or just don't catch and let the try-catch in launch_gui handle it.

For now, improve the catch to be more diagnostic but don't change behavior: the exception will cause CLI11 to print the error to stderr (which in GUI mode goes to the console panel via CoutRedirect).

### 2. Drag-drop file demo in example 06

**File**: `examples/06_colors_and_files/main.cpp`

Add a note in the comment header about drag-drop support:

```cpp
// Drag-drop: any file widget also accepts files dragged from Explorer
```

No code changes needed — drag-drop already works in the widgets, just add documentation.

## Verification

```bash
cmake --build build_gui && cmake --build build_examples && cmake --build build && build/tests/Debug/cli_gui_tests.exe
```
