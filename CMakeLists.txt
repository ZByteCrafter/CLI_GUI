cmake_minimum_required(VERSION 3.16)
project(CLI11_GUI VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CLI_GUI_ENABLE_GUI "Build GUI support (Dear ImGui + GLFW)" ON)
option(CLI_GUI_BUILD_TESTS "Build tests" OFF)
option(CLI_GUI_BUILD_EXAMPLES "Build example programs" OFF)
option(CLI_GUI_FORCE_VENDOR "Force vendor/ over system packages" OFF)

# CLI11 (always required, header-only)
add_subdirectory(vendor/CLI11 EXCLUDE_FROM_ALL)

# Library target (header-only by default)
add_library(CLI_GUI INTERFACE)
add_library(CLI_GUI::CLI_GUI ALIAS CLI_GUI)
target_include_directories(CLI_GUI INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(CLI_GUI INTERFACE CLI11::CLI11)

# GUI support (conditional)
if(CLI_GUI_ENABLE_GUI)
    message(STATUS "CLI_GUI: GUI support ENABLED")
    target_compile_definitions(CLI_GUI INTERFACE CLI_GUI_HAS_GUI)

    # Vendor imgui
    add_subdirectory(vendor/imgui EXCLUDE_FROM_ALL)
    target_link_libraries(CLI_GUI INTERFACE imgui)
    target_include_directories(CLI_GUI INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui_backends
    )

    # Vendor glfw
    add_subdirectory(vendor/glfw EXCLUDE_FROM_ALL)
    target_link_libraries(CLI_GUI INTERFACE glfw)

    # Compiled GUI translation unit (includes imgui backends)
    add_library(CLI_GUI_Gui STATIC
        src/CLI_GUI.cpp
        vendor/imgui_backends/imgui_impl_glfw.cpp
        vendor/imgui_backends/imgui_impl_opengl3.cpp
    )
    target_link_libraries(CLI_GUI_Gui PUBLIC CLI_GUI imgui glfw)
    target_include_directories(CLI_GUI_Gui PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui_backends
    )
    target_link_libraries(CLI_GUI INTERFACE CLI_GUI_Gui)
endif()

# Tests
if(CLI_GUI_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Examples
if(CLI_GUI_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
