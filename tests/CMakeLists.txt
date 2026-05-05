add_executable(cli_gui_tests
    test_app.cpp
)
target_link_libraries(cli_gui_tests PRIVATE CLI_GUI::CLI_GUI)
target_include_directories(cli_gui_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_test(NAME cli_gui_tests COMMAND cli_gui_tests)
