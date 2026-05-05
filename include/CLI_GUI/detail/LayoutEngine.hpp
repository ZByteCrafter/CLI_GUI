#pragma once

#ifdef CLI_GUI_HAS_GUI

#include <CLI_GUI/CLI_GUI.hpp>
#include <imgui.h>
#include <string>
#include <sstream>
#include <atomic>
#include <mutex>
#include <queue>
#include <cstring>
#include <algorithm>
#include <vector>
#include <cstdio>

namespace CLI_GUI {
namespace detail {

struct ConsoleState {
    std::mutex mtx;
    std::queue<std::string> lines;
    std::atomic<bool> running{false};
    bool show_console = true;
    int console_height = 150;
    bool run_requested = false;
    bool quit_requested = false;

    void push_line(const std::string& line) {
        std::lock_guard<std::mutex> lock(mtx);
        lines.push(line);
    }
    std::string pop_line() {
        std::lock_guard<std::mutex> lock(mtx);
        if (lines.empty()) return "";
        auto s = lines.front();
        lines.pop();
        return s;
    }
};

inline void render_option(App& app, CLI::Option* opt) {
    const auto& meta = app.gui_meta(opt);
    auto& mut_meta = app.gui_meta(opt);  // mutable ref for per-option state
    std::string label = meta.label.empty() ? opt->get_name() : meta.label;
    WidgetType wt = meta.widget_type;

    if (wt == WidgetType::Auto) {
        if (opt->get_expected_min() == 0)
            wt = WidgetType::Checkbox;
        else
            wt = WidgetType::InputText;
    }

    switch (wt) {
        case WidgetType::Checkbox:
        case WidgetType::Toggle: {
            if (!mut_meta.initialized) {
                mut_meta.bool_state = !opt->results().empty();
                mut_meta.initialized = true;
            }
            ImGui::Checkbox(label.c_str(), &mut_meta.bool_state);
            break;
        }
        case WidgetType::InputText:
        case WidgetType::Password:
        case WidgetType::FileOpen:
        case WidgetType::FileSave:
        case WidgetType::DirPicker:
        case WidgetType::CodeEditor:
        case WidgetType::IpAddress: {
            if (!mut_meta.initialized && !opt->results().empty()) {
                std::strncpy(mut_meta.text_buf, opt->results()[0].c_str(), sizeof(mut_meta.text_buf) - 1);
                mut_meta.text_buf[sizeof(mut_meta.text_buf) - 1] = '\0';
                mut_meta.initialized = true;
            }
            ImGui::InputText(label.c_str(), mut_meta.text_buf, sizeof(mut_meta.text_buf));
            break;
        }
        case WidgetType::InputInt:
        case WidgetType::SpinInt: {
            if (!mut_meta.initialized && !opt->results().empty()) {
                try { mut_meta.int_state = std::stoi(opt->results()[0]); }
                catch (...) { mut_meta.int_state = 0; }
                mut_meta.initialized = true;
            }
            ImGui::InputInt(label.c_str(), &mut_meta.int_state);
            break;
        }
        case WidgetType::InputFloat:
        case WidgetType::SpinFloat: {
            if (!mut_meta.initialized && !opt->results().empty()) {
                try { mut_meta.float_state = std::stof(opt->results()[0]); }
                catch (...) { mut_meta.float_state = 0.0f; }
                mut_meta.initialized = true;
            }
            ImGui::InputFloat(label.c_str(), &mut_meta.float_state);
            break;
        }
        case WidgetType::SliderInt: {
            if (!mut_meta.initialized && !opt->results().empty()) {
                try { mut_meta.int_state = std::stoi(opt->results()[0]); }
                catch (...) { mut_meta.int_state = 0; }
                mut_meta.initialized = true;
            }
            int mn = meta.has_min ? static_cast<int>(meta.min_val) : 0;
            int mx = meta.has_max ? static_cast<int>(meta.max_val) : 100;
            ImGui::SliderInt(label.c_str(), &mut_meta.int_state, mn, mx);
            break;
        }
        case WidgetType::SliderFloat: {
            if (!mut_meta.initialized && !opt->results().empty()) {
                try { mut_meta.float_state = std::stof(opt->results()[0]); }
                catch (...) { mut_meta.float_state = 0.0f; }
                mut_meta.initialized = true;
            }
            float mn = meta.has_min ? static_cast<float>(meta.min_val) : 0.0f;
            float mx = meta.has_max ? static_cast<float>(meta.max_val) : 1.0f;
            ImGui::SliderFloat(label.c_str(), &mut_meta.float_state, mn, mx);
            break;
        }
        case WidgetType::Combo:
        case WidgetType::Radio:
        case WidgetType::ToggleGroup: {
            if (!mut_meta.initialized) {
                mut_meta.initialized = true;
                // Initialize combo_current from CLI results, if available
                if (!opt->results().empty()) {
                    auto& current_val = opt->results()[0];
                    for (size_t i = 0; i < meta.values.size(); ++i) {
                        if (meta.values[i] == current_val) {
                            mut_meta.combo_current = static_cast<int>(i);
                            break;
                        }
                    }
                }
            }
            if (!meta.values.empty()) {
                std::vector<const char*> items;
                items.reserve(meta.values.size());
                for (auto& v : meta.values) items.push_back(v.c_str());
                ImGui::Combo(label.c_str(), &mut_meta.combo_current,
                             items.data(), static_cast<int>(items.size()));
            }
            break;
        }
        case WidgetType::ColorRGB: {
            if (!mut_meta.initialized) {
                mut_meta.initialized = true;
                if (!opt->results().empty()) {
                    // Try to parse "r g b" or "#RRGGBB" from results
                    sscanf(opt->results()[0].c_str(), "%f %f %f",
                           &mut_meta.color3[0], &mut_meta.color3[1], &mut_meta.color3[2]);
                }
            }
            ImGui::ColorEdit3(label.c_str(), mut_meta.color3);
            break;
        }
        case WidgetType::ColorRGBA: {
            if (!mut_meta.initialized) {
                mut_meta.initialized = true;
                if (!opt->results().empty()) {
                    sscanf(opt->results()[0].c_str(), "%f %f %f %f",
                           &mut_meta.color4[0], &mut_meta.color4[1],
                           &mut_meta.color4[2], &mut_meta.color4[3]);
                }
            }
            ImGui::ColorEdit4(label.c_str(), mut_meta.color4);
            break;
        }
        // Not yet fully implemented — degrade to InputText
        case WidgetType::List:
        case WidgetType::MultiSelect:
        case WidgetType::Duration:
        case WidgetType::TagList: {
            if (!mut_meta.initialized && !opt->results().empty()) {
                std::strncpy(mut_meta.text_buf, opt->results()[0].c_str(), sizeof(mut_meta.text_buf) - 1);
                mut_meta.text_buf[sizeof(mut_meta.text_buf) - 1] = '\0';
                mut_meta.initialized = true;
            }
            ImGui::InputText(label.c_str(), mut_meta.text_buf, sizeof(mut_meta.text_buf));
            break;
        }
        default:
            ImGui::Text("%s", label.c_str());
            break;
    }
}

inline void render_options(App& app) {
    for (auto* opt : app.get_options()) {
        render_option(app, opt);
    }
}

inline void render_subcommands(App& app, ConsoleState& console) {
    auto& subs = app.get_subcommands();
    if (subs.empty()) return;

    ImGui::Separator();
    if (ImGui::BeginTabBar("Subcommands")) {
        // Root tab (app-level options)
        if (ImGui::BeginTabItem(app.get_name().empty() ? "Main" : app.get_name().c_str())) {
            render_options(app);
            ImGui::EndTabItem();
        }
        for (auto* sub : subs) {
            if (ImGui::BeginTabItem(sub->get_name().c_str())) {
                for (auto* opt : sub->get_options()) {
                    render_option(app, opt);
                }
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
}

inline void render_console(ConsoleState& console) {
    if (!console.show_console) return;
    ImGui::Spacing();
    if (ImGui::CollapsingHeader("Output", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::BeginChild("ConsoleOutput",
            ImVec2(0, static_cast<float>(console.console_height)),
            true, ImGuiWindowFlags_HorizontalScrollbar);
        std::string line;
        while (!(line = console.pop_line()).empty()) {
            ImGui::TextUnformatted(line.c_str());
        }
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
        ImGui::EndChild();
    }
}

inline void render_bottom_bar(App& app, ConsoleState& console) {
    ImGui::Spacing();
    ImGui::Separator();

    if (console.running) {
        ImGui::ProgressBar(app.get_progress(), ImVec2(-1, 0));
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            app.request_cancel();
        }
    } else {
        if (ImGui::Button("Run", ImVec2(120, 0))) {
            console.run_requested = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Quit", ImVec2(120, 0))) {
            console.quit_requested = true;
        }
    }
}

} // namespace detail
} // namespace CLI_GUI

#endif
