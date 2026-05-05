#pragma once

#ifdef CLI_GUI_HAS_GUI

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <string>
#include <stdexcept>

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

private:
    GLFWwindow* window_ = nullptr;
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

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

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
