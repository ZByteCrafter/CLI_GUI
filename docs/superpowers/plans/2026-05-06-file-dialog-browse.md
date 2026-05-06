# File Dialog Browse Button — Implementation Plan

> **Date**: 2026-05-06
> **Scope**: Add native `[Browse...]` button to FileOpen/FileSave/DirPicker widgets
> **Files**: `include/CLI_GUI/detail/FileDialog.hpp` (new), `include/CLI_GUI/detail/LayoutEngine.hpp` (modify)

## Goal

Add a `[Browse...]` button next to FileOpen/FileSave/DirPicker text fields that opens a platform-native file/folder selection dialog.

## Current State

FileOpen/FileSave/DirPicker all fall through to the same InputText case — pure text input, no browse capability.

## Design

### 1. New helper: `FileDialog.hpp`

Three platform-specific functions:

| Function | Behavior (Windows) | Other platforms |
|----------|-------------------|-----------------|
| `open_file_dialog()` | `GetOpenFileNameA` | return false |
| `save_file_dialog()` | `GetSaveFileNameA` | return false |
| `dir_picker_dialog()` | `SHBrowseForFolderA` | return false |

Each writes selected path to `buf` and returns true on success.

### 2. LayoutEngine change

Split FileOpen/FileSave/DirPicker from the InputText fall-through case. New layout:

```
[__________ text field (shortened) __________] [Browse...]
```

The text field width is reduced by ~70px to make room for the button on the same line.

### 3. Widget rendering logic

```cpp
case WidgetType::FileOpen:
case WidgetType::FileSave:
case WidgetType::DirPicker: {
    // init text_buf (same as InputText)
    // render shortened InputText
    // ImGui::SameLine
    // if [Browse...] -> call corresponding dialog
}
case WidgetType::InputText:
case WidgetType::Password:
case WidgetType::CodeEditor:
case WidgetType::IpAddress: {
    // unchanged — no button
}
```

## Platform support

| Platform | API | Library |
|----------|-----|---------|
| Windows | `GetOpenFileNameA` / `GetSaveFileNameA` / `SHBrowseForFolderA` | `commdlg.h`, `shlobj.h` |
| macOS | fallback (return false) | — |
| Linux | fallback (return false) | — |

## Verification

```bash
cmake --build build_gui        # must compile
cmake --build build_examples   # examples must compile (06_colors_and_files uses FileOpen/FileSave/DirPicker)
```
