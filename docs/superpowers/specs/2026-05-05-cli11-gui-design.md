# CLI11_GUI 设计规格说明

> **日期**：2026-05-05
> **状态**：已确认
> **摘要**：扩展 CLI11，使得 CLI 程序在无参数启动时自动弹出 GUI 窗口收集选项，带参数时保持原生 CLI 行为。

---

## 1. 项目目标

CLI11_GUI 是一个 C++ 库，扩展 [CLI11](https://github.com/CLIUtils/CLI11)，使得同一份代码同时支持：

- **CLI 模式**：通过命令行参数启动时，行为与原生 CLI11 完全一致。
- **GUI 模式**：无参数启动时（如双击桌面图标），自动弹出 GUI 窗口，用户通过控件填写所有选项，点击运行后执行原程序逻辑。

核心价值：**零或极少的代码改动**将现有 CLI 程序改造为支持 GUI。

---

## 2. 技术选型

| 决策项 | 选择 | 理由 |
|--------|------|------|
| C++ 标准 | C++17 | CLI11 要求 C++11+，C++17 提供 `std::optional`、`if constexpr`、`std::variant` 等便利特性 |
| GUI 框架 | Dear ImGui (+ GLFW + OpenGL3) | 即时模式，header+1cpp，极度轻量，跨平台，适合开发者工具 |
| 窗口/渲染后端 | GLFW | 纯 C，最轻量窗口+OpenGL上下文管理，ImGui 官方原生支持 |
| 依赖获取 | find_package → FetchContent → vendor（三级优先序） | 覆盖联网/离线/系统包管理三种场景 |
| 构建系统 | CMake 3.16+ | 跨平台标准，FetchContent 支持 |

---

## 3. 库结构

```
CLI11_GUI/
├── include/CLI_GUI/
│   ├── CLI_GUI.hpp          # 唯一公开入口头文件
│   ├── App.hpp              # CLI_GUI::App 包装类
│   ├── GuiLauncher.hpp      # GUI 启动器（由 CLI_GUI_HAS_GUI 宏守卫）
│   ├── WidgetMapper.hpp     # CLI11 类型 → GUI 控件映射
│   └── detail/
│       ├── BackendGLFW.hpp  # ImGui+GLFW 后端封装
│       └── LayoutEngine.hpp # 分组折叠面板布局引擎
├── src/
│   └── CLI_GUI.cpp          # GUI 编译单元（仅当 CLI_GUI_ENABLE_GUI=ON 编译）
├── vendor/                  # 离线依赖（提交到 git）
│   ├── CLI11/               #   CLI11 header-only 源码
│   ├── imgui/               #   Dear ImGui 源码
│   ├── imgui_backends/      #   GLFW + OpenGL3 后端
│   └── glfw/                #   GLFW 最小源码集
├── examples/                # 示例程序
│   ├── 01_basic/
│   ├── 02_subcommands/
│   ├── 03_custom_widgets/
│   └── 04_long_task/
├── tests/                   # 测试
│   ├── test_app.cpp
│   ├── test_widget_mapper.cpp
│   └── test_gui_integration.cpp
├── cmake/
│   ├── CLI_GUIConfig.cmake.in
│   └── VendorGLFW.cmake
├── scripts/
│   └── package_release.py   # 打包脚本
└── CMakeLists.txt
```

### 3.1 关键设计原则

- `CLI_GUI.hpp` 是唯一需要 `#include` 的文件。
- 当 GUI 支持未启用时，整个库退化为 header-only 的 CLI11 薄包装，**零 GUI 依赖**。
- GUI 相关代码由 `#ifdef CLI_GUI_HAS_GUI` 宏守卫，不启用时不引入任何 ImGui/GLFW 头文件。

---

## 4. 公开 API

### 4.1 最简示例

```cpp
#include <CLI_GUI/CLI_GUI.hpp>

int main(int argc, char** argv) {
    CLI_GUI::App app{"My Tool v1.0"};

    int count = 0;
    app.add_option("-c,--count", count, "迭代次数")
       ->check(CLI::Range(1, 100));

    bool verbose = false;
    app.add_flag("-v,--verbose", verbose, "详细输出");

    CLI_GUI_PARSE(app, argc, argv);

    // 变量已填充完毕，执行逻辑
    run_my_logic(count, verbose);
}
```

### 4.2 核心 API

| API | 说明 |
|-----|------|
| `CLI_GUI::App` | 继承 `CLI::App`，完全兼容 CLI11 所有 API |
| `CLI_GUI_PARSE(app, argc, argv)` | 核心宏：argc==1 启动 GUI，否则正常 CLI 解析 |
| `.gui_label("显示名")` | 覆盖 GUI 中该选项的显示标签 |
| `.gui_widget(WidgetType)` | 手动指定控件类型 |
| `.gui_group("组名")` | 将选项归入命名折叠面板 |
| `.gui_min(val)` / `.gui_max(val)` | 设置 Slider/Spin 的范围 |
| `.gui_values({...})` | 设置 Combo/Radio 的可选值 |
| `app.gui_title("标题")` | 设置窗口标题（可选） |
| `app.gui_size(800, 600)` | 设置窗口大小（可选） |

### 4.3 耗时任务 API

```cpp
// 回调在后台线程中执行，GUI 保持响应
app.set_callback([&]() {
    for (int i = 0; i < count; ++i) {
        process_file(i);
        app.update_progress(i * 1.0f / count);  // 更新进度条
    }
});
CLI_GUI_PARSE(app, argc, argv);
```

| API | 说明 |
|-----|------|
| `app.set_callback(fn)` | 设置耗时任务回调（后台线程执行） |
| `app.set_cancel_check(fn)` | 自定义取消条件（可选） |
| `app.update_progress(pct)` | 在回调中调用，更新 GUI 进度（0.0~1.0） |
| `app.gui_show_console(bool)` | 是否显示控制台输出面板（默认 true） |
| `app.gui_console_height(px)` | 输出面板高度（默认 150px） |

**线程模型**：
- 用户回调在 `std::thread` 中执行。
- 进度通过 `std::atomic<float>` 传递，GUI 主线程定时读取。
- 取消通过 `std::atomic<bool>` 实现。
- `std::cout` 输出通过线程安全队列重定向到 GUI 面板。

### 4.4 控制台输出捕获

- **完全透明**：用户正常使用 `std::cout` / `printf`，无需任何改动。
- **实现**：`std::cout.rdbuf()` 替换为自定义 streambuf，通过线程安全队列传递到 GUI 输出面板。
- **GUI 布局**：窗口底部嵌入可折叠/可调高度的只读输出面板。

### 4.5 Subcommand 处理

- 每个 CLI11 subcommand 在 GUI 中映射为一个 **Tab 标签页**。
- 顶层选项（非 subcommand 专属）放在默认第一个 Tab 或共享区域。

```cpp
auto* sub = app.add_subcommand("process", "处理数据");
sub->add_option("-i,--input", input, "输入文件");
// GUI 中 "process" 成为一个 Tab
```

---

## 5. 控件映射规则

### 5.1 自动推断规则

| CLI11 类型/约束 | 推断控件 |
|-----------------|----------|
| `add_flag(...)` | `Checkbox` |
| `add_option<int/long/short>(...)` | `InputInt` + `Spin` |
| `add_option<float/double>(...)` | `InputFloat` + `Spin` |
| `add_option<std::string>(...)` | `InputText` |
| + `check(CLI::Range(min,max))` | `SliderInt` / `SliderFloat` |
| + `check(CLI::ExistingFile)` | `InputText` + `FileOpen` 按钮 |
| + `check(CLI::ExistingDirectory)` | `InputText` + `DirPicker` 按钮 |
| + `check(CLI::NonexistentPath)` | `InputText` + `FileSave` 按钮 |
| + `transform(CLI::CheckedTransformer(...))` | `Combo`（下拉） |
| + `transform(CLI::AsSizeValue(true))` | `InputText` + 单位 `Combo` |
| `add_option<vector<T>>(...)` | `List`（动态列表 +/-） |
| `add_option<filesystem::path>(...)` | `InputText` + `FileOpen` |
| `expected(0, N)` 多参数 | N 个并排 `InputText` |

### 5.2 手动可控控件类型

`Checkbox`、`Toggle`、`InputText`、`InputInt`、`InputFloat`、`SliderInt`、`SliderFloat`、`SpinInt`、`SpinFloat`、`Combo`、`Radio`、`FileOpen`、`FileSave`、`DirPicker`、`ColorRGB`、`ColorRGBA`、`List`、`MultiSelect`、`Password`、`IpAddress`、`Duration`、`CodeEditor`、`TagList`、`ToggleGroup`

---

## 6. GUI 窗口布局

```
┌──────────────────────────────────────────┐
│  My Tool v1.0                    [_][□][X] │
├──────────────────────────────────────────┤
│  [Tab: main]  [Tab: process]  [...]        │  ← Subcommand Tab
│                                            │
│  ▼ 基本选项                                │  ← 可折叠面板（按 group 分组）
│    Count:  [____10____] [▲▼]               │
│    Output: [__________] [📂 Browse]        │
│                                            │
│  ▼ 高级选项                                │
│    ☑ Verbose                               │
│    Algorithm: [AES ▼]                      │
│  ...                                       │
├──────────────────────────────────────────┤
│  Output                                    │  ← 控制台输出面板（可折叠）
│  ──────────────────────────────            │
│  [INFO] 正在处理文件 1/10...               │
│  [DONE] 处理完成，耗时 3.2秒               │
├──────────────────────────────────────────┤
│  ████████████░░░░ 80%    [Cancel] [Run]    │  ← 进度条 + 按钮
└──────────────────────────────────────────┘
```

**布局规则**：
- 有 subcommand 时顶部渲染 Tab 栏。
- 选项按 `CLI::OptionGroup` 分组成可折叠面板。
- 底部固定控制台输出面板。
- 最底部为进度条 + Run/Cancel 按钮。

---

## 7. 构建系统

### 7.1 CMake 选项

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `CLI_GUI_ENABLE_GUI` | ON | 编译 GUI 支持（需要 ImGui+GLFW） |
| `CLI_GUI_BUILD_EXAMPLES` | OFF | 构建示例程序 |
| `CLI_GUI_BUILD_TESTS` | OFF | 构建测试 |
| `CLI_GUI_FORCE_VENDOR` | OFF | 强制使用 vendor/ 中的源码而非系统包/FetchContent |

### 7.2 依赖解析优先级

```
1. find_package (系统已安装)
2. FetchContent  (联网下载)
3. vendor/       (离线源码)
```

### 7.3 用户集成

```cmake
# 纯 CLI 模式（默认，零 GUI 依赖，header-only）
find_package(CLI_GUI)
target_link_libraries(my_app CLI_GUI::CLI_GUI)

# 带 GUI 模式
find_package(CLI_GUI COMPONENTS Gui)
target_link_libraries(my_app CLI_GUI::CLI_GUI)
```

离线环境只需拿到包含 `vendor/` 的完整仓库，`add_subdirectory` 即可。

### 7.4 发布包

- `CLI11_GUI-X.Y.Z-slim.tar.gz` — 仅本库源码（联网用户）
- `CLI11_GUI-X.Y.Z-full.tar.gz` — 含 `vendor/` 全部依赖（离线用户）

---

## 8. 错误处理与校验

| 场景 | CLI 模式 | GUI 模式 |
|------|---------|---------|
| 必填选项未填 | CLI11 报错退出 | 控件边框变红 + 悬停提示 + Run 按钮灰掉 |
| 数值超出 Range | CLI11 报错退出 | Slider/Spin 自动限制范围；手动超范围输入红色警告 |
| 文件不存在 | CLI11 报错退出 | 文件选择器保证存在；手动输入无效路径红色警告 |
| 类型转换失败 | CLI11 报错退出 | 输入过滤非法字符（如 Int 框不接受字母） |
| `->required()` | CLI11 标准行为 | 标签旁红色 `*`，校验失败提示"请修正 N 个错误" |

**原则**：GUI 模式优先**预防错误**（Slider 限制范围、FilePicker 保证存在），仅在无法预防时显示友好提示。

---

## 9. 测试策略

| 层级 | 内容 | 框架 | 需 GUI？ |
|------|------|------|----------|
| 单元测试 | App 包装、WidgetMapper 推断、类型擦除 | GoogleTest / Catch2 | 否 |
| CLI 集成 | argc≥2 时与原生 CLI11 行为一致 | GoogleTest | 否 |
| GUI 数据流 | 模拟控件值 → 变量填充 → 回调执行 | 自定义 mock | 否 |
| E2E | 实际 GUI 窗口渲染和交互 | 需 Xvfb/CI | 是（可选） |

核心逻辑（WidgetMapper、App、LayoutEngine）与渲染逻辑（ImGui+GLFW）**严格分离**，≥90% 测试可在无 GUI 环境运行。

---

## 10. 架构概览

```
用户 main()
  │
CLI_GUI::App app;
app.add_option(...)
CLI_GUI_PARSE(app, argc, argv)
  │
  ├── argc == 1?
  │     │
  │     ├── YES → GUI 路径
  │     │   ├─ ImGui + GLFW 窗口创建
  │     │   ├─ LayoutEngine 根据 options/groups/subcommands 渲染控件
  │     │   ├─ WidgetMapper 根据类型推断/手动覆写确定控件
  │     │   ├─ 用户交互、填充变量
  │     │   ├─ [可选] set_callback → 后台线程执行
  │     │   │    ├─ update_progress() → 进度条
  │     │   │    └─ cout 重定向 → 控制台面板
  │     │   └─ GUI 关闭，控制权返回 main
  │     │
  │     └── NO → CLI 路径
  │         └─ CLI::App::parse (原生 CLI11)
  │
  └── 变量已填充，用户代码继续执行
```

---

## 11. 非目标（YAGNI）

- **GUI 主题/美化系统**：使用 ImGui 默认样式，v0.1 不提供自定义主题。
- **保存/加载配置**：GUI 中填写的值不持久化，v0.1 不做配置文件导入导出。
- **远程/Web GUI**：不做基于 HTTP 的远程控制界面。
- **移动端**：不支持 Android/iOS。
- **多语言/国际化**：界面使用英文，v0.1 不做 i18n。

---

## 12. 风险与缓解

| 风险 | 缓解措施 |
|------|---------|
| ImGui+GLFW 在无 GPU/无窗口管理器环境 crash | GUI 启动前检测图形环境，无可用环境时回退到 CLI 模式并警告 |
| Windows 上 MSVC/MinGW 的 GLFW 编译复杂度 | vendor/ 中包含 CMake 预配置脚本，自动化交叉编译 |
| CLI11 版本不兼容 | 锁定 CLI11 版本范围 [2.3, 3.0)，vendor/ 中固定版本 |
| GUI 线程安全问题 | 所有 ImGui 操作在主线程，用户回调和进度更新通过 atomics 传递 |

---

## 13. 术语定义

| 术语 | 定义 |
|------|------|
| **CLI 模式** | 通过命令行参数启动，使用 CLI11 原生解析流程 |
| **GUI 模式** | argc==1（无参数）启动时，弹出 Dear ImGui 窗口收集输入 |
| **选项** | CLI11 的 Option（flag 或带值 option） |
| **位置参数** | CLI11 的 positional argument |
| **Subcommand** | CLI11 的 Subcommand，GUI 中映射为 Tab |
| **Option Group** | CLI11 的 OptionGroup，GUI 中映射为可折叠面板 |
