#pragma once
#include <CLI/CLI.hpp>
#include <CLI_GUI/WidgetType.hpp>
#include <CLI_GUI/WidgetMapper.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <atomic>

namespace CLI_GUI {

/// Per-option GUI metadata stored in App.
struct OptionGuiMeta {
    WidgetType widget_type = WidgetType::Auto;
    std::string label;
    std::string group_name;
    double min_val = 0.0;
    double max_val = 100.0;
    std::vector<std::string> values;
    bool has_min = false;
    bool has_max = false;
    // Per-option runtime state (not static!)
    bool initialized = false;       // has this option been init'd from CLI11 results?
    char text_buf[1024] = {};
    int combo_current = 0;
    bool bool_state = false;
    int int_state = 0;
    float float_state = 0.0f;
    float color3[3] = {1.0f, 1.0f, 1.0f};
    float color4[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    bool folder_mode = false;       // FileOrDir toggle: file=false, folder=true
};

/// Extended CLI::App that stores GUI metadata for each option.
class App : public CLI::App {
public:
    using CLI::App::App;

    /// Get GUI metadata for an option pointer (non-const access).
    OptionGuiMeta& gui_meta(CLI::Option* opt) { return option_meta_[opt]; }

    /// Get GUI metadata for an option pointer (const access).
    /// Uses the non-const pointer directly since the map key is CLI::Option*.
    /// Safe: find() does not modify the key.
    const OptionGuiMeta& gui_meta(const CLI::Option* opt) const {
        // Key type is CLI::Option*, so we must cast. This is safe because
        // unordered_map::find only compares keys, never modifies them.
        auto it = option_meta_.find(const_cast<CLI::Option*>(opt));
        if (it != option_meta_.end()) return it->second;
        static const OptionGuiMeta empty;
        return empty;
    }

    // App-level GUI settings
    App* gui_title(std::string t)     { gui_title_ = std::move(t); return this; }
    App* gui_size(int w, int h)       { gui_width_ = w; gui_height_ = h; return this; }
    App* gui_show_console(bool s)     { gui_show_console_ = s; return this; }
    App* gui_console_height(int h)    { gui_console_height_ = h; return this; }
    App* set_callback(std::function<void()> cb) { gui_callback_ = std::move(cb); return this; }
    App* set_main(std::function<void()> fn)      { gui_main_ = std::move(fn); return this; }

    // App-level accessors
    std::string gui_title() const     { return gui_title_.empty() ? (get_name().empty() ? get_description() : get_name()) : gui_title_; }
    int gui_width() const             { return gui_width_; }
    int gui_height() const            { return gui_height_; }
    bool gui_show_console() const     { return gui_show_console_; }
    int gui_console_height() const    { return gui_console_height_; }
    std::function<void()> gui_callback() const { return gui_callback_; }
    std::function<void()> gui_main() const      { return gui_main_; }

    // Access to metadata map (for LayoutEngine)
    std::unordered_map<CLI::Option*, OptionGuiMeta>& option_meta_map() { return option_meta_; }
    const std::unordered_map<CLI::Option*, OptionGuiMeta>& option_meta_map() const { return option_meta_; }

    // Update progress from worker thread (atomic, thread-safe)
    void update_progress(float pct) { progress_.store(pct, std::memory_order_release); }
    float get_progress() const { return progress_.load(std::memory_order_acquire); }

    // Cancel flag for worker thread
    void request_cancel() { cancelled_.store(true, std::memory_order_release); }
    bool is_cancelled() const { return cancelled_.load(std::memory_order_acquire); }
    void reset_cancel() { cancelled_.store(false, std::memory_order_release); }

private:
    std::unordered_map<CLI::Option*, OptionGuiMeta> option_meta_;
    std::string gui_title_;
    int gui_width_ = 800;
    int gui_height_ = 600;
    bool gui_show_console_ = true;
    int gui_console_height_ = 150;
    std::function<void()> gui_callback_;
    std::function<void()> gui_main_;
    std::atomic<float> progress_{0.0f};
    std::atomic<bool> cancelled_{false};
};

// ---- Free functions to set GUI metadata on CLI::Option pointers ----

inline CLI::Option* gui_label(CLI::Option* opt, const std::string& label, App& app) {
    app.gui_meta(opt).label = label;
    return opt;
}
inline CLI::Option* gui_widget(CLI::Option* opt, WidgetType wt, App& app) {
    app.gui_meta(opt).widget_type = wt;
    return opt;
}
inline CLI::Option* gui_group(CLI::Option* opt, const std::string& g, App& app) {
    app.gui_meta(opt).group_name = g;
    return opt;
}
inline CLI::Option* gui_min(CLI::Option* opt, double v, App& app) {
    app.gui_meta(opt).min_val = v;
    app.gui_meta(opt).has_min = true;
    return opt;
}
inline CLI::Option* gui_max(CLI::Option* opt, double v, App& app) {
    app.gui_meta(opt).max_val = v;
    app.gui_meta(opt).has_max = true;
    return opt;
}
inline CLI::Option* gui_values(CLI::Option* opt, std::vector<std::string> vals, App& app) {
    app.gui_meta(opt).values = std::move(vals);
    return opt;
}

// ---- CLI_GUI_PARSE macro ----

#ifdef CLI_GUI_HAS_GUI
#define CLI_GUI_PARSE(app, argc, argv)                    \
    do {                                                   \
        if ((argc) <= 1) {                                 \
            CLI_GUI::launch_gui((app), (argc), (argv));    \
        } else {                                           \
            try { (app).parse((argc), (argv)); }           \
            catch (const CLI::ParseError& e) {              \
                (app).exit(e);                              \
            }                                               \
            auto __cb = (app).gui_callback();              \
            if (__cb) { __cb(); }                          \
            else {                                         \
                auto __main = (app).gui_main();            \
                if (__main) __main();                      \
            }                                              \
        }                                                   \
    } while (0)
#else
#define CLI_GUI_PARSE(app, argc, argv)                    \
    do {                                                   \
        try { (app).parse((argc), (argv)); }               \
        catch (const CLI::ParseError& e) {                  \
            (app).exit(e);                                  \
        }                                                   \
        auto __cb = (app).gui_callback();                  \
        if (__cb) { __cb(); }                              \
        else {                                             \
            auto __main = (app).gui_main();                \
            if (__main) __main();                          \
        }                                                  \
    } while (0)
#endif

} // namespace CLI_GUI
