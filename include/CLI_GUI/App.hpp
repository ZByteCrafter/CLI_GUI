#pragma once
#include <CLI/CLI.hpp>
#include <string>
#include <vector>

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

/// Wrapper around CLI::App with GUI metadata.
class App : public CLI::App {
public:
    using CLI::App::App;
};

/// Macro: argc==1 launches GUI; otherwise normal CLI11 parse.
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
        }                                                   \
    } while (0)
#else
#define CLI_GUI_PARSE(app, argc, argv)                    \
    do {                                                   \
        try { (app).parse((argc), (argv)); }               \
        catch (const CLI::ParseError& e) {                  \
            (app).exit(e);                                  \
        }                                                   \
    } while (0)
#endif

} // namespace CLI_GUI
