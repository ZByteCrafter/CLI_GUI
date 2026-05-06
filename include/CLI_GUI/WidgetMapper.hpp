#pragma once

#include <string>
#include <type_traits>
#include <vector>
#include <CLI_GUI/WidgetType.hpp>

namespace CLI_GUI {

inline constexpr const char* to_string(WidgetType wt) {
    switch (wt) {
        case WidgetType::Auto:        return "Auto";
        case WidgetType::Checkbox:    return "Checkbox";
        case WidgetType::Toggle:      return "Toggle";
        case WidgetType::InputText:   return "InputText";
        case WidgetType::InputInt:    return "InputInt";
        case WidgetType::InputFloat:  return "InputFloat";
        case WidgetType::SliderInt:   return "SliderInt";
        case WidgetType::SliderFloat: return "SliderFloat";
        case WidgetType::SpinInt:     return "SpinInt";
        case WidgetType::SpinFloat:   return "SpinFloat";
        case WidgetType::Combo:       return "Combo";
        case WidgetType::Radio:       return "Radio";
        case WidgetType::FileOpen:    return "FileOpen";
        case WidgetType::FileSave:    return "FileSave";
        case WidgetType::DirPicker:   return "DirPicker";
        case WidgetType::FileOrDir:   return "FileOrDir";
        case WidgetType::ColorRGB:    return "ColorRGB";
        case WidgetType::ColorRGBA:   return "ColorRGBA";
        case WidgetType::List:        return "List";
        case WidgetType::MultiSelect: return "MultiSelect";
        case WidgetType::Password:    return "Password";
        case WidgetType::IpAddress:   return "IpAddress";
        case WidgetType::Duration:    return "Duration";
        case WidgetType::CodeEditor:  return "CodeEditor";
        case WidgetType::TagList:     return "TagList";
        case WidgetType::ToggleGroup: return "ToggleGroup";
    }
    return "Unknown";
}

namespace detail {
    template <typename T, typename = void>
    struct widget_type_impl {
        static constexpr WidgetType value = WidgetType::Auto;
    };

    template <>
    struct widget_type_impl<bool> {
        static constexpr WidgetType value = WidgetType::Checkbox;
    };

    template <typename T>
    struct widget_type_impl<T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>>> {
        static constexpr WidgetType value = WidgetType::InputInt;
    };

    template <typename T>
    struct widget_type_impl<T, std::enable_if_t<std::is_floating_point_v<T>>> {
        static constexpr WidgetType value = WidgetType::InputFloat;
    };

    template <>
    struct widget_type_impl<std::string> {
        static constexpr WidgetType value = WidgetType::InputText;
    };

    template <typename T>
    struct widget_type_impl<std::vector<T>> {
        static constexpr WidgetType value = WidgetType::List;
    };
} // namespace detail

template <typename T>
constexpr WidgetType widget_type_for() {
    return detail::widget_type_impl<T>::value;
}

} // namespace CLI_GUI
