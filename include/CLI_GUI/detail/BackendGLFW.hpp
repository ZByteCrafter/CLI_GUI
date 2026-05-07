#pragma once

#ifdef CLI_GUI_HAS_GUI

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <mutex>
#include <stdexcept>
#include <cstdio>

namespace CLI_GUI {
namespace detail {

/// Manages ImGui + GLFW + OpenGL3 lifecycle.
class BackendGLFW {
public:
    BackendGLFW(const std::string& title, int width, int height);
    ~BackendGLFW();

    bool should_close() const { return glfwWindowShouldClose(window_); }
    void new_frame();
    void render();

    GLFWwindow* window() { return window_; }

    /// Poll and consume file paths dropped onto the window since last call.
    static std::vector<std::string> take_dropped_paths() {
        std::lock_guard<std::mutex> lock(s_drop_mutex_);
        auto paths = s_dropped_paths_;
        s_dropped_paths_.clear();
        return paths;
    }

private:
    GLFWwindow* window_ = nullptr;

    static inline std::mutex s_drop_mutex_;
    static inline std::vector<std::string> s_dropped_paths_;

    static void drop_callback(GLFWwindow*, int count, const char** paths) {
        std::lock_guard<std::mutex> lock(s_drop_mutex_);
        for (int i = 0; i < count; ++i)
            s_dropped_paths_.emplace_back(paths[i]);
    }
};

BackendGLFW::BackendGLFW(const std::string& title, int width, int height) {
    if (!glfwInit())
        throw std::runtime_error("CLI_GUI: Failed to initialize GLFW");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window_) {
        glfwTerminate();
        throw std::runtime_error("CLI_GUI: Failed to create GLFW window");
    }

    // Register file drop callback for drag-drop support
    glfwSetDropCallback(window_, drop_callback);

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);

    // Set a default window icon (32x32, blue-purple gradient)
    {
        static unsigned char icon_pixels[32 * 32 * 4];
        static bool icon_built = false;
        if (!icon_built) {
            for (int y = 0; y < 32; ++y) {
                for (int x = 0; x < 32; ++x) {
                    int i = (y * 32 + x) * 4;
                    icon_pixels[i+0] = 220;                           // B
                    icon_pixels[i+1] = (unsigned char)(50 + y*6);    // G
                    icon_pixels[i+2] = (unsigned char)(180 + x*2);   // R
                    icon_pixels[i+3] = 255;                           // A
                }
            }
            icon_built = true;
        }
        GLFWimage img;
        img.width  = 32;
        img.height = 32;
        img.pixels = icon_pixels;
        glfwSetWindowIcon(window_, 1, &img);
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // Try to load a CJK font for Chinese/Japanese/Korean support
    ImGuiIO& io = ImGui::GetIO();
    bool cjk_loaded = false;
    const char* cjk_font_paths[] = {
        "C:/Windows/Fonts/msyh.ttc",                   // Microsoft YaHei (Win)
        "C:/Windows/Fonts/simsun.ttc",                 // SimSun (Win)
        "/System/Library/Fonts/PingFang.ttc",           // macOS
        "/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc", // Linux
    };
    for (auto* path : cjk_font_paths) {
        FILE* fp = std::fopen(path, "rb");
        if (fp) {
            std::fclose(fp);
            io.Fonts->AddFontFromFileTTF(path, 16.0f, nullptr,
                io.Fonts->GetGlyphRangesChineseFull());
            cjk_loaded = true;
            break;
        }
    }
    if (!cjk_loaded) {
        io.Fonts->AddFontDefault();  // fallback to built-in
    }

    // Wrap backend init so that if either fails, ImGui context + GLFW
    // are properly cleaned up rather than leaked.
    try {
        if (!ImGui_ImplGlfw_InitForOpenGL(window_, true))
            throw std::runtime_error("ImGui_ImplGlfw_InitForOpenGL failed");
        if (!ImGui_ImplOpenGL3_Init("#version 150"))
            throw std::runtime_error("ImGui_ImplOpenGL3_Init failed");
    } catch (...) {
        ImGui::DestroyContext();
        glfwDestroyWindow(window_);
        window_ = nullptr;
        glfwTerminate();
        throw;
    }
}

BackendGLFW::~BackendGLFW() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
    glfwTerminate();
}

void BackendGLFW::new_frame() {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void BackendGLFW::render() {
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window_, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.18f, 0.20f, 0.22f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window_);
}

} // namespace detail
} // namespace CLI_GUI

#endif // CLI_GUI_HAS_GUI
