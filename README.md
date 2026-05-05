# CLI11_GUI

> 扩展 CLI11，同一份代码同时支持 CLI 和 GUI。

当程序通过**命令行参数**启动时，行为与原生 [CLI11](https://github.com/CLIUtils/CLI11) 完全一致。当**无参数**启动时（双击桌面图标），自动弹出 GUI 窗口，以可视化控件收集所有选项后执行原程序逻辑。

---

## 快速开始

代码只需 **4 处改动**：

```cpp
#include <CLI_GUI/CLI_GUI.hpp>          // 1. 换头文件
#include <CLI_GUI/detail/Win32SuppressConsole.hpp>  // (Windows) 抑制双击黑框

int main(int argc, char** argv) {
    cli_gui_init_console();             // (Windows) 必须在首行
    CLI_GUI::App app{"My Tool"};        // 2. CLI::App → CLI_GUI::App

    int count = 0;
    app.add_option("-c,--count", count, "迭代次数");
    bool verbose = false;
    app.add_flag("-v,--verbose", verbose, "详细输出");

    app.set_main([&]() {                // 3. 注册主逻辑
        run_my_logic(count, verbose);   //    在回调中访问填充好的变量
    });

    CLI_GUI_PARSE(app, argc, argv);     // 4. 换宏
}
```

```bash
./my_tool -c 10 -v       # CLI 模式
./my_tool                 # GUI 模式（双击）
```

---

## 如何集成到你的项目

### 你需要哪些文件

| 模式 | 需要的文件 | 说明 |
|------|-----------|------|
| **带 GUI**（推荐） | 整个仓库 `CLI11_GUI/` | 通过 `add_subdirectory` 引入，vendor/ 已含全部离线依赖 |
| **仅 CLI** | `include/CLI_GUI/` + CLI11 | header-only，零编译。自行安装 CLI11 或从 vendor/ 获取 |

### 方式一：`add_subdirectory`（推荐，支持 GUI + 离线）

```cmake
# 将 CLI11_GUI 仓库放到你的项目目录下，例如 third_party/CLI11_GUI/
add_subdirectory(third_party/CLI11_GUI)
target_link_libraries(my_app CLI_GUI::CLI_GUI)

# 需要 GUI 时确保 CMake 默认开启:
# set(CLI_GUI_ENABLE_GUI ON)   # 默认就是 ON
```

如果你的项目有图标资源，还需要链接 `.rc` 文件（Windows）：

```cmake
if(MSVC)
    target_sources(my_app PRIVATE resources/my_icon.rc)
    set_target_properties(my_app PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
    )
endif()
```

### 方式二：`cmake --install` + `find_package`（仅 header-only）

```bash
cd CLI11_GUI
cmake -B build -DCLI_GUI_ENABLE_GUI=OFF
cmake --install build --prefix /usr/local
```

```cmake
find_package(CLI_GUI)
target_link_libraries(my_app CLI_GUI::CLI_GUI)
```

> **注意**：安装模式仅支持 header-only（无 GUI）。GUI 模式请用 `add_subdirectory`。

### 方式三：只拷贝头文件（极简）

如果你只需要 CLI 模式，拷贝 `include/CLI_GUI/` 到你的项目即可。需要自行提供 CLI11（`#include <CLI/CLI.hpp>` 能找到即可）。

---

## API 参考

### App 方法

```cpp
CLI_GUI::App app{"My Tool"};        // 构造（参数 = 描述；双参数 = 描述, 名称）

// GUI 设置
app.gui_title("窗口标题");           // 窗口标题栏文字（默认用 App 名称或描述）
app.gui_size(1024, 768);            // 窗口大小（默认 800×600）
app.gui_show_console(false);        // 隐藏控制台输出面板
app.gui_console_height(200);        // 控制台面板高度（默认 150px）

// 主逻辑
app.set_main(fn);                   // 注册主逻辑：GUI 在线程中执行，CLI 在 parse 后执行
app.set_callback(fn);               // 注册长耗时回调（带进度条 + Cancel 按钮）

// 回调中使用
app.update_progress(0.5f);          // 更新进度条（0.0 ~ 1.0）
app.is_cancelled();                 // 检查 Cancel 是否被按下
```

### 选项元数据函数

```cpp
CLI::Option* opt = app.add_option("-n,--name", val, "描述");

CLI_GUI::gui_label(opt, "显示标签", app);                         // 覆写 GUI 标签
CLI_GUI::gui_widget(opt, CLI_GUI::WidgetType::SliderInt, app);    // 手动指定控件
CLI_GUI::gui_group(opt, "组名", app);                             // 手动指定分组
CLI_GUI::gui_min(opt, 0, app);                                    // 最小值（Slider/Spin）
CLI_GUI::gui_max(opt, 100, app);                                  // 最大值（Slider/Spin）
CLI_GUI::gui_values(opt, {"a","b","c"}, app);                     // 候选值（Combo/Radio）
```

### `set_main` vs `set_callback`

```cpp
// 方式一：set_main — 简单输出，推荐大多数场景
app.set_main([&]() {
    std::cout << "Hello, " << name << "!" << std::endl;
});
CLI_GUI_PARSE(app, argc, argv);
// CLI 模式：parse 后同步执行   GUI 模式：Run 触发，在线程中执行，输出在控制台面板

// 方式二：set_callback — 长耗时 + 进度条
app.set_callback([&]() {
    for (int i = 1; i <= steps; ++i) {
        if (app.is_cancelled()) return;            // 响应 Cancel
        std::cout << "Step " << i << std::endl;
        app.update_progress((float)i / steps);
    }
});
CLI_GUI_PARSE(app, argc, argv);
// CLI 模式：只解析参数（不执行回调）   GUI 模式：Run 触发，带进度条
```

### `CLI_GUI_PARSE` 宏

```cpp
// 展开为:
// if (argc <= 1)  →  launch_gui(app, argc, argv);   // GUI 模式
// else           →  app.parse(argc, argv);           // CLI 模式
//                   if (gui_main) gui_main();         // 执行 set_main
```

---

## 支持的控件类型

| 类别 | 控件 |
|------|------|
| 输入 | `InputText` `InputInt` `InputFloat` `Password` `CodeEditor` |
| 开关 | `Checkbox` `Toggle` |
| 滑块 | `SliderInt` `SliderFloat` `SpinInt` `SpinFloat` |
| 选择 | `Combo` `Radio` `ToggleGroup` `MultiSelect` |
| 文件 | `FileOpen` `FileSave` `DirPicker` |
| 颜色 | `ColorRGB` `ColorRGBA` |
| 集合 | `List` `TagList` |
| 其他 | `IpAddress` `Duration` |

> **自动推断**：`int` → `InputInt`，`bool` → `Checkbox`，`std::string` → `InputText`，`std::vector<T>` → `List`。
> 可通过 `gui_widget()` 手动覆写任意控件类型。

---

## 示例程序

| 示例 | 说明 | 演示特性 |
|------|------|----------|
| `01_basic` | Hello GUI | 基本选项、SliderInt、Checkbox |
| `02_subcommands` | 数据分析 | `add_subcommand` → Tab 标签页 |
| `03_custom_widgets` | 自定义控件 | Password、Radio、CodeEditor、FileSave |
| `04_long_task` | 长耗时任务 | `set_callback`、进度条、Cancel |
| `05_option_groups` | 图片处理器 | `add_option_group` → 折叠面板 |
| `06_colors_and_files` | 主题设计器 | ColorRGB/RGBA、FileOpen/Save/DirPicker |
| `07_positional_and_vectors` | 批量重命名 | 位置参数、vector 选项 |
| `08_full_app` | 媒体转换套件 | 综合示例（子命令+分组+回调+颜色） |

运行：

```bash
# CLI 模式
build/examples/Debug/01_basic.exe -n "CLI" -c 3 -u

# GUI 模式（双击或直接运行——无黑框）
build/examples/Debug/01_basic.exe
```

---

## 构建

```bash
# 仅 CLI（header-only）
cmake -B build -DCLI_GUI_ENABLE_GUI=OFF

# 带 GUI（编译 ImGui + GLFW）
cmake -B build_gui -DCLI_GUI_ENABLE_GUI=ON
cmake --build build_gui

# 运行 33 个测试
cmake -B build -DCLI_GUI_BUILD_TESTS=ON -DCLI_GUI_ENABLE_GUI=OFF
cmake --build build
build/tests/Debug/cli_gui_tests.exe

# 构建全部 8 个示例
cmake -B build_examples -DCLI_GUI_ENABLE_GUI=ON -DCLI_GUI_BUILD_EXAMPLES=ON
cmake --build build_examples
```

| CMake 选项 | 默认 | 说明 |
|-----------|------|------|
| `CLI_GUI_ENABLE_GUI` | ON | 编译 GUI 支持（Dear ImGui + GLFW） |
| `CLI_GUI_BUILD_TESTS` | OFF | 构建单元测试 |
| `CLI_GUI_BUILD_EXAMPLES` | OFF | 构建示例程序 |
| `CLI_GUI_FORCE_VENDOR` | OFF | 强制使用 vendor/ |

---

## 目录结构

```
CLI11_GUI/
├── include/CLI_GUI/          # ★ 公开头文件——这是你需要 include 的
│   ├── CLI_GUI.hpp           #   唯一入口：#include <CLI_GUI/CLI_GUI.hpp>
│   ├── App.hpp               #   App 类 + OptionGuiMeta + CLI_GUI_PARSE 宏
│   ├── WidgetType.hpp        #   WidgetType 枚举（25 种）
│   ├── WidgetMapper.hpp      #   编译期类型 → 控件映射
│   └── detail/               #   内部实现（不需要直接 include）
├── src/CLI_GUI.cpp           # GUI 编译单元（仅 GUI 模式编译）
├── vendor/                   # ★ 离线依赖——add_subdirectory 时自动编译
│   ├── CLI11/                #   CLI11 v2.4 header-only
│   ├── imgui/                #   Dear ImGui v1.91
│   ├── imgui_backends/       #   GLFW + OpenGL3 后端
│   └── glfw/                 #   GLFW 3.4
├── tests/                    # 33 个单元测试
├── examples/                 # 8 个完整示例
├── resources/                # Windows 资源文件（.ico + .rc）
├── cmake/                    # CMake 包配置
└── CMakeLists.txt
```

---

## 技术栈

- **C++17** — `if constexpr` / `constexpr` / `std::atomic`
- **CLI11 v2.4** — header-only CLI 解析（vendor 内嵌）
- **Dear ImGui v1.91** — 即时模式 GUI 框架（可选）
- **GLFW 3.4 + OpenGL3** — 窗口和渲染后端（可选）
- **CMake 3.16+** — 支持 `install` / `find_package` / `add_subdirectory`

---

## License

MIT
