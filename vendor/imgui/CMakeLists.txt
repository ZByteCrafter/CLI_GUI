cmake_minimum_required(VERSION 3.16)
project(imgui_vendor LANGUAGES CXX)

add_library(imgui STATIC
    imgui.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
)
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
