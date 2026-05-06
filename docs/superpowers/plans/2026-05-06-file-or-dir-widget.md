# FileOrDir Widget — Implementation Plan

> **Date**: 2026-05-06
> **Scope**: New `WidgetType::FileOrDir` combining file and folder selection with a toggle
> **Files**: 5 modified

## Goal

Add a composite widget that lets users pick either a file or a folder from a single control, toggled by a checkbox.

## Design

```
Path: [__________________________] [Browse]  ☐ Folder
       ↑ text_buf (path)           ↑ file dialog  ↑ folder mode toggle
```

## Changes

### 1. `include/CLI_GUI/WidgetType.hpp`
Add `FileOrDir` to the `WidgetType` enum (after `DirPicker`).

### 2. `include/CLI_GUI/App.hpp`
Add `bool folder_mode = false;` to `OptionGuiMeta` struct.

### 3. `include/CLI_GUI/WidgetMapper.hpp`
Add `case WidgetType::FileOrDir: return "FileOrDir";` to `to_string()`.

### 4. `include/CLI_GUI/detail/LayoutEngine.hpp`
Add `case WidgetType::FileOrDir:` alongside `FileOpen`/`FileSave`/`DirPicker`, with an extra `ImGui::Checkbox("Folder", &mut_meta.folder_mode)` before the Browse button. The Browse dialog reads `folder_mode` to decide file vs folder.

### 5. `src/CLI_GUI.cpp`
Add `case WidgetType::FileOrDir:` to `flush_gui_to_cli` switch (same as InputText: read `text_buf`).

## Verification
```bash
cmake --build build_gui        # compile
cmake --build build_examples   # compile
cmake --build build            # tests still pass
```
