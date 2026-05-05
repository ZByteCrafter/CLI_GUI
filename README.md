# CLI11_GUI

> 扩展 CLI11，同一份代码同时支持 CLI 和 GUI。

当程序通过命令行参数启动时，行为与原生 [CLI11](https://github.com/CLIUtils/CLI11) 完全一致。当**无参数**启动时（如双击桌面图标），自动弹出 GUI 窗口，以可视化控件收集所有选项后执行原程序逻辑。

---

## 快速开始

### 代码只需三处改动

```cpp
#include <CLI_GUI/CLI_GUI.hpp>   // 1. 换头文件

int main(int argc, char** argv) {
    CLI_GUI::App app{"My Tool"};  // 2. CLI::App → CLI_GUI::App

    int count = 0;
    app.add_option("-c,--count", count, "迭代次数");
    bool verbose = false;
    app.add_flag("-v,--verbose", verbose, "详细输出");

    CLI_GUI_PARSE(app, argc, argv);  // 3. 换宏

    // 变量已填充，无论 CLI 还是 GUI
    run_my_logic(count, verbose);
}
```

```bash
# CLI 模式（有参数）
./my_tool -c 10 -v

# GUI 模式（无参数 / 双击）
./my_tool
```

### CMake 集成

```cmake
# 纯 CLI（默认，零 GUI 依赖）
find_package(CLI_GUI)
target_link_libraries(my_app CLI_GUI::CLI_GUI)

# 带 GUI
find_package(CLI_GUI COMPONENTS Gui)
target_link_libraries(my_app CLI_GUI::CLI_GUI)
```

离线环境直接 `add_subdirectory(CLI11_GUI)`，vendor/ 已包含全部依赖。

---

## 特性

| 特性 | 说明 |
|------|------|
| **CLI/GUI 双模式** | argc≤1 自动弹 GUI，≥2 走 CLI11 原生解析 |
| **零依赖 header-only** | `CLI_GUI_ENABLE_GUI=OFF` 时退化为纯 CLI11 包装 |
| **自动控件推断** | `int`→SpinInt, `bool`→Checkbox, `string`→InputText, `vector`→List... |
| **手动控件覆写** | `.gui_widget(WidgetType::SliderInt)` 强制指定控件类型 |
| **子命令** | GUI 中通过 Tab 标签页切换 |
| **分组折叠面板** | 利用 CLI11 的 OptionGroup 自动分组 |
| **控制台输出捕获** | `std::cout` / `printf` 自动重定向到 GUI 输出面板 |
| **耗时任务** | `set_callback` 在后台线程执行，进度条 + Cancel |
| **离线可用** | vendor/ 包含 CLI11 + Dear ImGui + GLFW 全部源码 |
| **GPU 回退** | 无图形环境时自动降级为 CLI 模式 |

---

## 支持的控件类型

`Checkbox` `Toggle` `InputText` `InputInt` `InputFloat`
`SliderInt` `SliderFloat` `SpinInt` `SpinFloat`
`Combo` `Radio` `FileOpen` `FileSave` `DirPicker`
`ColorRGB` `ColorRGBA` `List` `MultiSelect`
`Password` `IpAddress` `Duration` `CodeEditor` `TagList` `ToggleGroup`

---

## 高级用法

### 子命令

```cpp
auto* sub = app.add_subcommand("process", "处理数据");
sub->add_option("-i,--input", input, "输入文件");
// GUI 中 "process" 成为一个 Tab
```

### 耗时任务 + 进度条

```cpp
app.set_callback([&]() {
    for (int i = 1; i <= count; ++i) {
        std::cout << "处理 " << i << "/" << count << "..." << std::endl;
        app.update_progress(i * 1.0f / count);
    }
});
CLI_GUI_PARSE(app, argc, argv);
```

### GUI 元数据

```cpp
int count = 0;
auto* n_opt = app.add_option("-c,--count", count, "迭代次数");

// 设置元数据 —— 通过独立函数
CLI_GUI::gui_label(n_opt, "迭代次数", app);
CLI_GUI::gui_widget(n_opt, CLI_GUI::WidgetType::SliderInt, app);
CLI_GUI::gui_min(n_opt, 1, app);
CLI_GUI::gui_max(n_opt, 100, app);

// 下拉框 / 单选按钮
std::string format;
auto* f_opt = app.add_option("-f,--format", format, "输出格式");
CLI_GUI::gui_widget(f_opt, CLI_GUI::WidgetType::Combo, app);
CLI_GUI::gui_values(f_opt, {"json","csv","xml","txt"}, app);

// 文件保存对话框
std::string output;
auto* o_opt = app.add_option("-o,--output", output, "输出文件");
CLI_GUI::gui_widget(o_opt, CLI_GUI::WidgetType::FileSave, app);

// App 级设置
app.gui_title("My Tool v2.0");
app.gui_size(1024, 768);
```

---

## 构建

```bash
# 仅 CLI（header-only，无需编译）
cmake -B build -DCLI_GUI_ENABLE_GUI=OFF
cmake --build build

# 带 GUI（需要编译 ImGui + GLFW）
cmake -B build -DCLI_GUI_ENABLE_GUI=ON
cmake --build build

# 运行测试
cmake -B build -DCLI_GUI_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build

# 构建示例
cmake -B build -DCLI_GUI_ENABLE_GUI=ON -DCLI_GUI_BUILD_EXAMPLES=ON
cmake --build build
```

**CMake 选项：**

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `CLI_GUI_ENABLE_GUI` | ON | 编译 GUI 支持 (Dear ImGui + GLFW) |
| `CLI_GUI_BUILD_TESTS` | OFF | 构建单元测试 |
| `CLI_GUI_BUILD_EXAMPLES` | OFF | 构建示例程序 |
| `CLI_GUI_FORCE_VENDOR` | OFF | 强制使用 vendor/ 而非系统包 |

---

## 技术栈

- **C++17** — 现代 C++，`if constexpr` / `std::optional` / `std::variant`
- **CLI11** — header-only CLI 解析库
- **Dear ImGui** — 即时模式 GUI 框架（可选）
- **GLFW + OpenGL3** — 窗口和渲染后端（可选）
- **CMake 3.16+** — 跨平台构建系统
- **vendor/** — 离线依赖，无需网络即可构建

---

## 目录结构

```
CLI11_GUI/
├── include/CLI_GUI/      # 公开头文件
│   ├── CLI_GUI.hpp       #   唯一公开入口
│   ├── App.hpp           #   CLI_GUI::App 包装类 + CLI_GUI_PARSE 宏
│   ├── WidgetType.hpp    #   WidgetType 枚举
│   ├── WidgetMapper.hpp  #   编译期类型→控件映射
│   ├── GuiLauncher.hpp   #   启动 GUI 的函数声明
│   └── detail/           #   内部实现 (BackendGLFW, LayoutEngine, ConsoleCapture)
├── src/CLI_GUI.cpp       # GUI 编译单元
├── vendor/               # 离线依赖 (CLI11 + ImGui + GLFW)
├── tests/                # 单元测试 (23 个)
├── examples/             # 示例程序 (4 个)
│   ├── 01_basic/         #   基础选项
│   ├── 02_subcommands/   #   子命令
│   ├── 03_custom_widgets/#   自定义控件
│   └── 04_long_task/     #   耗时任务 + 进度条
└── CMakeLists.txt
```

---

## License

MIT
