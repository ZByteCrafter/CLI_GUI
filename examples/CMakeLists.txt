function(add_cli_gui_example name)
    add_executable(${name} ${name}/main.cpp)
    target_link_libraries(${name} PRIVATE CLI_GUI::CLI_GUI)
endfunction()

add_cli_gui_example(01_basic)
add_cli_gui_example(02_subcommands)
add_cli_gui_example(03_custom_widgets)
add_cli_gui_example(04_long_task)
