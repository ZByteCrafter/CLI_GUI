# CLI11_GUI

> 扩展 CLI11，同一份代码同时支持 CLI 和 GUI。

当程序通过**命令行参数**启动时，行为与原生 [CLI11](https://github.com/CLIUtils/CLI11) 完全一致。当**无参数**启动时（如双击桌面图标），自动弹出 GUI 窗口，以可视化控件收集所有选项后执行原程序逻辑。

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
# 纯 CLI（header-only，零 GUI 依赖）
find_package(CLI_GUI)
target_link_libraries(my_app CLI_GUI::CLI_GUI)

# 带 GUI
add_subdirectory(CLI11_GUI)  # vendor/ 包含全部依赖
target_link_libraries(my_app CLI_GUI::CLI_GUI)
```

> **离线环境**：直接 `add_subdirectory(CLI11_GUI)` 即可，`vendor/` 已包含 CLI11 + Dear ImGui + GLFW 全部源码。亦可 `cmake --install` 后使用 `find_package`（仅 header-only 模式）。

---

## 特性

| 特性 | 说明 |
|------|------|
| **CLI/GUI 双模式** | argc≤1 自动弹 GUI，≥2 走 CLI11 原生解析 |
| **零依赖 header-only** | `CLI_GUI_ENABLE_GUI=OFF` 时退化为纯 CLI11 包装 |
| **自动控件推断** | `int`→SpinInt，`bool`→Checkbox，`string`→InputText，`vector`→List… |
| **手动控件覆写** | `CLI_GUI::gui_widget(opt, WidgetType::SliderInt, app)` |
| **子命令** | GUI 中通过 Tab 标签页切换 |
| **OptionGroup 折叠面板** | CLI11 的 `add_option_group()` 自动渲染为可折叠区域 |
| **控制台输出捕获** | `std::cout` / `printf` 自动重定向到 GUI 输出面板 |
| **耗时任务** | `set_callback` 在后台线程执行，进度条 + Cancel 按钮 |
| **颜色选择器** | `ColorRGB` / `ColorRGBA` 控件（ImGui `ColorEdit3/4`） |
| **文件对话框** | `FileOpen` / `FileSave` / `DirPicker` 控件 |
| **离线可用** | `vendor/` 包含全部依赖，无需网络 |
| **GPU 回退** | 无图形环境时自动降级为 CLI 模式（`app.parse`） |

---

## 支持的控件类型

| 类别 | 控件 |
|------|------|
| 基本输入 | `InputText` `InputInt` `InputFloat` `Password` `CodeEditor` |
| 开关 | `Checkbox` `Toggle` |
| 滑块/微调 | `SliderInt` `SliderFloat` `SpinInt` `SpinFloat` |
| 选择 | `Combo` `Radio` `ToggleGroup` `MultiSelect` |
| 文件/目录 | `FileOpen` `FileSave` `DirPicker` |
| 颜色 | `ColorRGB` `ColorRGBA` |
| 集合 | `List` `TagList` |
| 特殊 | `IpAddress` `Duration` |

---

## API 参考

### App 方法

```cpp
app.gui_title("窗口标题");       // 设置 GUI 窗口标题（默认使用 App 名称）
app.gui_size(1024, 768);        // 设置 GUI 窗口大小（默认 800x600）
app.gui_show_console(false);    // 隐藏控制台输出面板
app.gui_console_height(200);    // 设置控制台面板高度
app.set_callback(fn);           // 设置 Run 按钮回调（后台线程执行）
app.update_progress(0.5f);      // 更新进度条（0.0~1.0，回调中调用）
app.is_cancelled();             // 检查 Cancel 是否被按下
```

### 选项元数据函数

```cpp
CLI::Option* opt = app.add_option("-n,--name", val, "desc");

CLI_GUI::gui_label(opt, "显示标签", app);          // 覆写 GUI 标签
CLI_GUI::gui_widget(opt, CLI_GUI::WidgetType::SliderInt, app);  // 手动指定控件
CLI_GUI::gui_group(opt, "组名", app);              // 手动指定分组
CLI_GUI::gui_min(opt, 0, app);                     // 最小值（Slider/Spin）
CLI_GUI::gui_max(opt, 100, app);                   // 最大值（Slider/Spin）
CLI_GUI::gui_values(opt, {"a","b","c"}, app);      // 候选值（Combo/Radio）
```

### 耗时任务回调

```cpp
app.set_callback([&]() {
    for (int i = 1; i <= steps; ++i) {
        if (app.is_cancelled()) return;             // 响应取消
        std::cout << "Step " << i << std::endl;      // 输出到 GUI 控制台
        app.update_progress((float)i / steps);       // 更新进度
    }
});
CLI_GUI_PARSE(app, argc, argv);  // GUI 模式：Run 触发回调；CLI 模式：直接解析
```

---

## 示例程序

| 示例 | 说明 | 演示特性 |
|------|------|----------|
| `01_basic` | Hello GUI | 基本选项、SliderInt、Checkbox、CLI/GUI 双模式 |
| `02_subcommands` | 数据分析工具 | `add_subcommand` → Tab 标签页、Combo 选择 |
| `03_custom_widgets` | 自定义控件 | Password、Radio、CodeEditor、FileSave |
| `04_long_task` | 长耗时任务 | `set_callback`、进度条、Cancel、控制台输出 |
| `05_option_groups` | 图片处理器 | `add_option_group` → 折叠面板、滑块、Combo |
| `06_colors_and_files` | 主题设计器 | ColorRGB/RGBA、FileOpen/Save、DirPicker、Password |
| `07_positional_and_vectors` | 批量重命名 | 位置参数、vector 选项、Required 校验 |
| `08_full_app` | 媒体转换套件 | 综合示例：子命令 + OptionGroup + 回调 + 颜色 + 文件 |

运行任一示例：

```bash
# CLI 模式
build/examples/Debug/01_basic.exe -n "CLI" -c 3 -u

# GUI 模式（双击或直接运行）
build/examples/Debug/01_basic.exe
```

---

## 构建

```bash
# 仅 CLI（header-only，无需编译）
cmake -B build -DCLI_GUI_ENABLE_GUI=OFF
cmake --build build

# 带 GUI（需要编译 ImGui + GLFW）
cmake -B build_gui -DCLI_GUI_ENABLE_GUI=ON
cmake --build build_gui

# 运行测试（33 个）
cmake -B build -DCLI_GUI_BUILD_TESTS=ON -DCLI_GUI_ENABLE_GUI=OFF
cmake --build build
build/tests/Debug/cli_gui_tests.exe

# 构建所有示例
cmake -B build_ex -DCLI_GUI_ENABLE_GUI=ON -DCLI_GUI_BUILD_EXAMPLES=ON
cmake --build build_ex

# 安装（仅 header-only 模式）
cmake --install build --prefix /usr/local
```

**CMake 选项：**

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `CLI_GUI_ENABLE_GUI` | ON | 编译 GUI 支持（Dear ImGui + GLFW） |
| `CLI_GUI_BUILD_TESTS` | OFF | 构建单元测试 |
| `CLI_GUI_BUILD_EXAMPLES` | OFF | 构建示例程序 |
| `CLI_GUI_FORCE_VENDOR` | OFF | 强制使用 `vendor/` 而非系统包 |

---

## 技术栈

- **C++17** — `if constexpr` / `constexpr` / `std::atomic` / 折叠表达式
- **CLI11 v2.4** — header-only CLI 解析库（vendor 内嵌）
- **Dear ImGui v1.91** — 即时模式 GUI 框架（可选，vendor 内嵌）
- **GLFW 3.4 + OpenGL3** — 窗口和渲染后端（可选，vendor 内嵌）
- **CMake 3.16+** — 跨平台构建系统，支持 `install`/`find_package`/`add_subdirectory`

---

## 目录结构

```
CLI11_GUI/
├── include/CLI_GUI/          # 公开头文件
│   ├── CLI_GUI.hpp           #   唯一公开入口
│   ├── App.hpp               #   App 类 + OptionGuiMeta + CLI_GUI_PARSE
│   ├── WidgetType.hpp        #   WidgetType 枚举（25 种）
│   ├── WidgetMapper.hpp      #   编译期类型 → 控件映射
│   ├── GuiLauncher.hpp       #   launch_gui() 声明
│   └── detail/               #   内部实现
│       ├── BackendGLFW.hpp   #     ImGui + GLFW + OpenGL3 生命周期
│       ├── LayoutEngine.hpp  #     表单渲染（控件/分组/子命令/控制台）
│       └── ConsoleCapture.hpp#     cout → GUI 面板重定向
├── src/CLI_GUI.cpp           # GUI 编译单元（flush_gui_to_cli + launch_gui）
├── vendor/                   # 离线依赖
│   ├── CLI11/                #   CLI11 v2.4 header-only
│   ├── imgui/                #   Dear ImGui v1.91
│   ├── imgui_backends/       #   GLFW + OpenGL3 后端
│   └── glfw/                 #   GLFW 3.4（最小文件集）
├── tests/                    # 测试（33 个）
├── examples/                 # 示例程序（8 个）
├── cmake/                    # CMake 包配置
└── CMakeLists.txt
```

---

## License

MIT
