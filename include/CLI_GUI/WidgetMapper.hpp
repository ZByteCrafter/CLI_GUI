#pragma once

#include <string>

namespace CLI_GUI {

// Forward-declared WidgetType -- full definition in WidgetMapper.hpp (Task 3).
// This stub allows CLI_GUI.hpp to compile before WidgetMapper is implemented.
enum class WidgetType {
    Auto = 0,
    Checkbox,
    Toggle,
    InputText,
    InputInt,
    InputFloat,
    SliderInt,
    SliderFloat,
    SpinInt,
    SpinFloat,
    Combo,
    Radio,
    FileOpen,
    FileSave,
    DirPicker,
    ColorRGB,
    ColorRGBA,
    List,
    MultiSelect,
    Password,
    IpAddress,
    Duration,
    CodeEditor,
    TagList,
    ToggleGroup
};

inline const char* to_string(WidgetType wt) {
    return "WidgetType";
}

} // namespace CLI_GUI
