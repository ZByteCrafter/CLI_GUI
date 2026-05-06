#pragma once

#ifdef CLI_GUI_HAS_GUI

#include <CLI_GUI/CLI_GUI.hpp>
#include <imgui.h>
#include <string>
#include <sstream>
#include <atomic>
#include <mutex>
#include <deque>
#include <cstring>
#include <algorithm>
#include <vector>
#include <set>
#include <cstdio>
#include <chrono>
#include <fstream>

namespace CLI_GUI {
namespace detail {

struct ConsoleState {
    static constexpr size_t kMaxLines = 1000;

    mutable std::mutex mtx;
    std::deque<std::string> buffer;
    std::atomic<bool> running{false};
    bool show_console = true;
    int console_height = 150;
    bool run_requested = false;
    bool quit_requested = false;
    std::string active_subcommand;  // name of the selected subcommand tab
    bool auto_scroll = true;        // auto-scroll to bottom on new output

    /// Append a line. If over limit, drop oldest.
    void push_line(const std::string& line) {
        std::lock_guard<std::mutex> lock(mtx);
        buffer.push_back(line);
        while (buffer.size() > kMaxLines) {
            buffer.pop_front();
        }
    }

    /// Return a snapshot of all buffered lines (thread-safe copy).
    std::deque<std::string> snapshot() const {
        std::lock_guard<std::mutex> lock(mtx);
        return buffer;
    }

    /// Clear all buffered lines (thread-safe).
    void clear() {
        std::lock_guard<std::mutex> lock(mtx);
        buffer.clear();
    }
};

/// Detect log level from a line prefix and return the display color.
inline ImVec4 detect_log_level(const std::string& line) {
    if (line.compare(0, 7, "[ERROR]") == 0 || line.compare(0, 7, "[FATAL]") == 0)
        return ImVec4(1.0f, 0.20f, 0.20f, 1.0f);   // red
    if (line.compare(0, 6, "[WARN]")  == 0 || line.compare(0, 9, "[WARNING]") == 0)
        return ImVec4(1.0f, 0.70f, 0.10f, 1.0f);   // orange
    if (line.compare(0, 6, "[DONE]")  == 0 || line.compare(0, 4, "[OK]") == 0)
        return ImVec4(0.15f, 0.90f, 0.25f, 1.0f);   // green
    if (line.compare(0, 8, "[CANCEL]") == 0)
        return ImVec4(0.50f, 0.50f, 0.50f, 1.0f);   // gray
    if (line.compare(0, 6, "[INFO]")  == 0 || line.compare(0, 7, "[DEBUG]") == 0)
        return ImVec4(0.55f, 0.70f, 1.0f, 1.0f);   // light blue
    return ImVec4(0.90f, 0.90f, 0.90f, 1.0f);        // light gray (default)
}

inline void render_option(App& app, CLI::Option* opt) {
    const auto& meta = app.gui_meta(opt);
    auto& mut_meta = app.gui_meta(opt);

    // Skip CLI11's default help flag in GUI — no terminal to display it
    std::string raw_name = opt->get_name();
    if (raw_name.find("--help") != std::string::npos ||
        raw_name.find(",-h")  != std::string::npos) {
        return;
    }

    std::string label = meta.label.empty() ? raw_name : meta.label;
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
            if (!mut_meta.initialized) {
                if (!opt->results().empty()) {
                    std::strncpy(mut_meta.text_buf, opt->results()[0].c_str(), sizeof(mut_meta.text_buf) - 1);
                    mut_meta.text_buf[sizeof(mut_meta.text_buf) - 1] = '\0';
                }
                mut_meta.initialized = true;
            }
            ImGui::InputText(label.c_str(), mut_meta.text_buf, sizeof(mut_meta.text_buf));
            break;
        }
        case WidgetType::InputInt:
        case WidgetType::SpinInt: {
            if (!mut_meta.initialized) {
                if (!opt->results().empty()) {
                    try { mut_meta.int_state = std::stoi(opt->results()[0]); }
                    catch (...) { mut_meta.int_state = 0; }
                }
                mut_meta.initialized = true;
            }
            ImGui::InputInt(label.c_str(), &mut_meta.int_state);
            break;
        }
        case WidgetType::InputFloat:
        case WidgetType::SpinFloat: {
            if (!mut_meta.initialized) {
                if (!opt->results().empty()) {
                    try { mut_meta.float_state = std::stof(opt->results()[0]); }
                    catch (...) { mut_meta.float_state = 0.0f; }
                }
                mut_meta.initialized = true;
            }
            ImGui::InputFloat(label.c_str(), &mut_meta.float_state);
            break;
        }
        case WidgetType::SliderInt: {
            if (!mut_meta.initialized) {
                if (!opt->results().empty()) {
                    try { mut_meta.int_state = std::stoi(opt->results()[0]); }
                    catch (...) { mut_meta.int_state = 0; }
                }
                mut_meta.initialized = true;
            }
            int mn = meta.has_min ? static_cast<int>(meta.min_val) : 0;
            int mx = meta.has_max ? static_cast<int>(meta.max_val) : 100;
            ImGui::SliderInt(label.c_str(), &mut_meta.int_state, mn, mx);
            break;
        }
        case WidgetType::SliderFloat: {
            if (!mut_meta.initialized) {
                if (!opt->results().empty()) {
                    try { mut_meta.float_state = std::stof(opt->results()[0]); }
                    catch (...) { mut_meta.float_state = 0.0f; }
                }
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

/// Render all options from an App, grouped by CLI::OptionGroup into collapsible sections.
/// Options without a group render outside any collapsible header.
inline void render_options(App& app) {
    auto groups = app.get_groups();
    std::set<std::string> rendered_groups;

    // Render options belonging to named groups (collapsible)
    for (auto& gname : groups) {
        if (gname.empty()) continue;
        bool open = ImGui::CollapsingHeader(gname.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
        // Collect options in this group (from both root and subcommand options)
        // Actually we need to iterate over app's options twice:
        // first for non-grouped, then for each group. More efficient: one pass.
        // For simplicity, we just iterate over groups and render matching options.
        if (open) {
            for (auto* opt : app.get_options()) {
                if (opt->get_group() == gname) {
                    render_option(app, opt);
                    rendered_groups.insert(gname);
                }
            }
        } else {
            // Still mark as rendered so they don't appear in the ungrouped fallthrough
            rendered_groups.insert(gname);
        }
    }

    // Render options with no group (or whose group wasn't in get_groups())
    for (auto* opt : app.get_options()) {
        if (opt->get_group().empty() || rendered_groups.count(opt->get_group()) == 0) {
            render_option(app, opt);
        }
    }
}

inline void render_subcommands(App& app, ConsoleState& console) {
    // Use filter overload — iterates registered subcommands, not parsed ones
    auto subs = app.get_subcommands([](CLI::App*) { return true; });
    if (subs.empty()) return;

    ImGui::Separator();
    if (ImGui::BeginTabBar("Subcommands")) {
        // Root tab (app-level options)
        bool root_open = ImGui::BeginTabItem(
            app.get_name().empty() ? "Main" : app.get_name().c_str());
        if (root_open) {
            render_options(app);
            ImGui::EndTabItem();
        }
        if (root_open) {
            console.active_subcommand.clear();  // root tab = no subcommand
        }

        for (auto* sub : subs) {
            bool tab_open = ImGui::BeginTabItem(sub->get_name().c_str());
            if (tab_open) {
                for (auto* opt : sub->get_options()) {
                    render_option(app, opt);
                }
                ImGui::EndTabItem();
                console.active_subcommand = sub->get_name();
            }
        }
        ImGui::EndTabBar();
    }
}

inline void render_console(ConsoleState& console) {
    if (!console.show_console) return;
    ImGui::Spacing();

    if (ImGui::CollapsingHeader("Output", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto snapshot = console.snapshot();

        // Toolbar row — right-aligned: Auto-scroll checkbox + Copy/Clear buttons
        ImGui::SetCursorPosX(
            ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - 230);
        ImGui::Checkbox("Auto-scroll", &console.auto_scroll);
        ImGui::SameLine();
        if (ImGui::SmallButton("Save")) {
            auto now = std::chrono::system_clock::now();
            auto t = std::chrono::system_clock::to_time_t(now);
            std::tm tm_buf;
#ifdef _WIN32
            localtime_s(&tm_buf, &t);
#else
            localtime_r(&t, &tm_buf);
#endif
            char name[64];
            std::strftime(name, sizeof(name), "log_%Y%m%d_%H%M%S.txt", &tm_buf);
            std::ofstream ofs(name);
            if (ofs) {
                for (auto& line : snapshot) { ofs << line << "\n"; }
                console.push_line("[DONE] Log saved to " + std::string(name));
            } else {
                console.push_line("[ERROR] Failed to save log file");
            }
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Copy")) {
            std::string all;
            for (auto& line : snapshot) {
                all += line;
                all += "\n";
            }
            ImGui::SetClipboardText(all.c_str());
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Clear")) { console.clear(); }

        ImGui::BeginChild("ConsoleOutput",
            ImVec2(0, static_cast<float>(console.console_height)),
            true, ImGuiWindowFlags_HorizontalScrollbar);
        for (auto& line : snapshot) {
            ImGui::TextColored(detect_log_level(line), "%s", line.c_str());
        }
        if (console.auto_scroll &&
            ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
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
