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
        case WidgetType::Checkbox: {
            // Init from results on first frame
            static bool first_frame = true;
            if (first_frame || mut_meta.text_buf[0] == 0) {
                mut_meta.bool_state = !opt->results().empty();
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
            if (mut_meta.text_buf[0] == 0 && !opt->results().empty())
                std::strncpy(mut_meta.text_buf, opt->results()[0].c_str(), sizeof(mut_meta.text_buf)-1);
            ImGui::InputText(label.c_str(), mut_meta.text_buf, sizeof(mut_meta.text_buf));
            break;
        }
        case WidgetType::InputInt:
        case WidgetType::SpinInt: {
            if (mut_meta.text_buf[0] == 0 && !opt->results().empty())
                mut_meta.int_state = std::stoi(opt->results()[0]);
            ImGui::InputInt(label.c_str(), &mut_meta.int_state);
            break;
        }
        case WidgetType::InputFloat:
        case WidgetType::SpinFloat: {
            if (mut_meta.text_buf[0] == 0 && !opt->results().empty())
                mut_meta.float_state = std::stof(opt->results()[0]);
            ImGui::InputFloat(label.c_str(), &mut_meta.float_state);
            break;
        }
        case WidgetType::SliderInt: {
            if (mut_meta.text_buf[0] == 0 && !opt->results().empty())
                mut_meta.int_state = std::stoi(opt->results()[0]);
            int mn = meta.has_min ? static_cast<int>(meta.min_val) : 0;
            int mx = meta.has_max ? static_cast<int>(meta.max_val) : 100;
            ImGui::SliderInt(label.c_str(), &mut_meta.int_state, mn, mx);
            break;
        }
        case WidgetType::SliderFloat: {
            if (mut_meta.text_buf[0] == 0 && !opt->results().empty())
                mut_meta.float_state = std::stof(opt->results()[0]);
            float mn = meta.has_min ? static_cast<float>(meta.min_val) : 0.0f;
            float mx = meta.has_max ? static_cast<float>(meta.max_val) : 1.0f;
            ImGui::SliderFloat(label.c_str(), &mut_meta.float_state, mn, mx);
            break;
        }
        case WidgetType::Combo:
        case WidgetType::Radio: {
            if (!meta.values.empty()) {
                const char* items[32];
                int n = std::min((int)meta.values.size(), 32);
                for (int i = 0; i < n; ++i) items[i] = meta.values[i].c_str();
                ImGui::Combo(label.c_str(), &mut_meta.combo_current, items, n);
            }
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
