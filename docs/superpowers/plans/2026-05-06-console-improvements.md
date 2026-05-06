# Console Output Improvements — Implementation Plan

> **Date**: 2026-05-06
> **Scope**: Color-coded log levels + Clear button in GUI console panel
> **Files**: `include/CLI_GUI/detail/LayoutEngine.hpp`

## Goal

Improve the GUI console panel with:
1. Color-coded log levels (INFO/WARN/ERROR/DONE/CANCEL)
2. A "Clear" button to flush console history

## Changes

### 1. ConsoleState::clear() (3 lines added)

After `ConsoleState::snapshot()`, add:

```cpp
void clear() {
    std::lock_guard<std::mutex> lock(mtx);
    buffer.clear();
}
```

### 2. detect_log_level() helper (12 lines added)

After `ConsoleState` struct, add a free inline function that maps line prefixes to ImVec4 colors:

| Prefix | Color | Hex |
|--------|-------|-----|
| `[ERROR]` `[FATAL]` | Red | `#FF3333` |
| `[WARN]` `[WARNING]` | Orange | `#FFB21A` |
| `[DONE]` `[OK]` | Green | `#33FF4D` |
| `[CANCEL]` | Gray | `#808080` |
| `[INFO]` `[DEBUG]` | Light blue | `#B3CCFF` |
| default | White | `#FFFFFF` |

### 3. render_console() rewrite (replace ~15 lines)

- `CollapsingHeader` return value stored for panel visibility
- `ImGui::SameLine` + `ImGui::SmallButton("Clear")` in header row
- `ImGui::TextUnformatted` → `ImGui::TextColored`, color from `detect_log_level()`

## Verification

```bash
cmake --build build_gui        # GUI build must succeed
cmake --build build_examples   # Examples must compile
cmake --build build            # Tests must still pass (33/33)
```
