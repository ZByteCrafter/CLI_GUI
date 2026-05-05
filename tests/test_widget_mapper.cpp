#include "catch2.hpp"
#include <CLI_GUI/WidgetMapper.hpp>
#include <string>
#include <vector>

TEST_CASE("WidgetType from int infers InputInt", "[widget_mapper]") {
    REQUIRE(CLI_GUI::widget_type_for<int>() == CLI_GUI::WidgetType::InputInt);
}

TEST_CASE("WidgetType from double infers InputFloat", "[widget_mapper]") {
    REQUIRE(CLI_GUI::widget_type_for<double>() == CLI_GUI::WidgetType::InputFloat);
}

TEST_CASE("WidgetType from string infers InputText", "[widget_mapper]") {
    REQUIRE(CLI_GUI::widget_type_for<std::string>() == CLI_GUI::WidgetType::InputText);
}

TEST_CASE("WidgetType from bool infers Checkbox", "[widget_mapper]") {
    REQUIRE(CLI_GUI::widget_type_for<bool>() == CLI_GUI::WidgetType::Checkbox);
}

TEST_CASE("WidgetType from vector infers List", "[widget_mapper]") {
    REQUIRE(CLI_GUI::widget_type_for<std::vector<int>>() == CLI_GUI::WidgetType::List);
}

TEST_CASE("WidgetType from long infers InputInt", "[widget_mapper]") {
    REQUIRE(CLI_GUI::widget_type_for<long>() == CLI_GUI::WidgetType::InputInt);
}

TEST_CASE("WidgetType from float infers InputFloat", "[widget_mapper]") {
    REQUIRE(CLI_GUI::widget_type_for<float>() == CLI_GUI::WidgetType::InputFloat);
}

TEST_CASE("to_string converts WidgetType to name", "[widget_mapper]") {
    REQUIRE(CLI_GUI::to_string(CLI_GUI::WidgetType::Checkbox) == std::string("Checkbox"));
    REQUIRE(CLI_GUI::to_string(CLI_GUI::WidgetType::InputInt) == std::string("InputInt"));
    REQUIRE(CLI_GUI::to_string(CLI_GUI::WidgetType::Auto) == std::string("Auto"));
}
