function(add_cli_gui_example name)
    add_executable(${name} ${name}/main.cpp)
    target_link_libraries(${name} PRIVATE CLI_GUI::CLI_GUI)
    if(MSVC)
        set_target_properties(${name} PROPERTIES
            LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
        )
        target_sources(${name} PRIVATE
            ${CMAKE_SOURCE_DIR}/resources/cli_gui.rc
        )
    endif()
endfunction()

add_cli_gui_example(01_basic)
add_cli_gui_example(02_subcommands)
add_cli_gui_example(03_custom_widgets)
add_cli_gui_example(04_long_task)
add_cli_gui_example(05_option_groups)
add_cli_gui_example(06_colors_and_files)
add_cli_gui_example(07_positional_and_vectors)
add_cli_gui_example(08_full_app)
add_cli_gui_example(09_positional)
