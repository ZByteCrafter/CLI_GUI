# Remaining Widgets + Improvements — Implementation Plan

> **Date**: 2026-05-06
> **Scope**: Duration, List, TagList, MultiSelect widgets, flush_gui_to_cli tests, drag-drop files, FileOrDir docs

## Items

### 1. Duration widget
- **Storage**: reuse existing `int_state` + `combo_current`
- **LayoutEngine**: SpinInt + unit Combo (s/m/h)
- **flush_gui_to_cli**: same as InputInt (single value)
- **No new OptionGuiMeta fields needed**

### 2. List widget
- **Storage**: new `std::vector<std::string> list_items` in OptionGuiMeta
- **LayoutEngine**: dynamic rows with [+] [-] buttons
- **flush_gui_to_cli**: repeat option name for each item
- **Example usage**: 07_positional_and_vectors already uses vector options

### 3. TagList widget
- **Storage**: same `list_items` as List
- **LayoutEngine**: InputText + Enter to add chip, X to remove
- **flush_gui_to_cli**: same as List (repeat option name)

### 4. MultiSelect widget
- **Storage**: new `std::vector<bool> selected_flags` in OptionGuiMeta; uses `meta.values` for labels
- **LayoutEngine**: checkbox list from values
- **flush_gui_to_cli**: repeat option name for each selected item

### 5. flush_gui_to_cli tests
- New test file `tests/test_flush.cpp`
- Test: set up text_buf/bool_state → call flush → verify variables set correctly
- Test: List/MultiSelect repeat-name flushing
- Test: subcommand flushing

### 6. Drag-drop file support
- FileOpen/FileSave/DirPicker/FileOrDir: accept drag-drop
- `ImGui::BeginDragDropTarget()`, check payload, write to text_buf

### 7. FileOrDir in README
- Add FileOrDir to widget types table and API reference

## Build verification
```bash
cmake --build build_gui && cmake --build build_examples && cmake --build build && build/tests/Debug/cli_gui_tests.exe
```
