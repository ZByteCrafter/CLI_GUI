#pragma once

namespace CLI_GUI {

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
    FileOrDir,
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

} // namespace CLI_GUI
