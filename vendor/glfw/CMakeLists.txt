cmake_minimum_required(VERSION 3.16)
project(glfw_vendor LANGUAGES C)

add_library(glfw STATIC
    src/context.c src/init.c src/input.c src/monitor.c
    src/platform.c src/vulkan.c src/window.c
    src/egl_context.c src/osmesa_context.c
    src/null_init.c src/null_joystick.c src/null_monitor.c src/null_window.c
)

if(WIN32)
    target_sources(glfw PRIVATE
        src/wgl_context.c src/win32_init.c src/win32_joystick.c
        src/win32_module.c src/win32_monitor.c src/win32_thread.c
        src/win32_time.c src/win32_window.c
    )
    target_compile_definitions(glfw PRIVATE _GLFW_WIN32)
    target_link_libraries(glfw PUBLIC opengl32)
elseif(APPLE)
    target_sources(glfw PRIVATE
        src/cocoa_init.m src/cocoa_joystick.m src/cocoa_monitor.m
        src/cocoa_window.m src/cocoa_time.c src/nsgl_context.m
        src/posix_module.c src/posix_thread.c
    )
    target_link_libraries(glfw PUBLIC "-framework Cocoa" "-framework OpenGL" "-framework IOKit")
else()
    target_sources(glfw PRIVATE
        src/glx_context.c src/x11_init.c src/x11_monitor.c
        src/x11_window.c src/xkb_unicode.c
        src/linux_joystick.c src/posix_module.c src/posix_thread.c
        src/posix_time.c src/posix_poll.c
    )
    target_link_libraries(glfw PUBLIC X11 dl pthread)
endif()

target_include_directories(glfw PUBLIC include PRIVATE src)
